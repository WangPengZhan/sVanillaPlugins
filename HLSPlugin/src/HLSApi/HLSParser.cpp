#include "HLSParser.h"

#include <limits>

#include "HLSConstants.h"
#include "Util/NumberParse.h"

namespace hlsapi
{
namespace
{
std::string_view tagOf(std::string_view line)
{
    return line.substr(0, line.find(':'));
}
}  // namespace

bool HLSParser::parse(std::string_view content)
{
    m_state = State::None;
    m_playlist = {};
    m_master = {};
    m_activeKey = {};
    m_activeKey.strMethod = key_method_none;
    m_unparsedTags.clear();
    m_previousRangeUri.clear();
    m_previousRangeEnd = 0;
    m_hasPreviousRange = false;
    m_valid = true;

    bool sawHeader = false;
    size_t start = 0;
    while (start <= content.size())
    {
        const size_t end = content.find_first_of("\r\n", start);
        const auto line = trim(content.substr(start, end - start));
        if (!line.empty())
        {
            if (!sawHeader)
            {
                sawHeader = (line == ext_m3u);
                m_valid = sawHeader;
            }
            else
            {
                parseLine(line);
            }
        }

        if (end == std::string_view::npos)
        {
            break;
        }

        start = end + 1;
        if (content[end] == '\r' && start < content.size() && content[start] == '\n')
        {
            ++start;
        }
    }

    const bool pendingUri = m_state != State::None;
    const bool mixed = (!m_master.streams.empty() || !m_master.mediaInfos.empty()) && (!m_playlist.segments.empty() || m_playlist.hasMap);
    const bool hasContent = !m_master.streams.empty() || !m_playlist.segments.empty();
    if (!sawHeader || pendingUri || mixed || !hasContent || !m_valid)
    {
        m_playlist = {};
        m_master = {};
        m_unparsedTags.clear();
        m_state = State::None;
        return false;
    }
    return true;
}

void HLSParser::parseLine(std::string_view line)
{
    if (!m_valid)
    {
        return;
    }
    if (!line.starts_with('#'))
    {
        bool assigned = false;
        if (m_state == State::WaitingForStreamUri && !m_master.streams.empty())
        {
            m_master.streams.back().uri = std::string(line);
            assigned = !m_master.streams.back().getUri().empty();
        }
        else if (m_state == State::WaitingForSegmentUri && !m_playlist.segments.empty())
        {
            auto& segment = m_playlist.segments.back();
            segment.uri = std::string(line);
            assigned = !segment.getUri().empty();
            if (assigned && segment.byteRangeLength != 0)
            {
                const auto resolvedUri = segment.getUri();
                if (!segment.hasByteRangeOffset)
                {
                    if (!m_hasPreviousRange || resolvedUri != m_previousRangeUri)
                    {
                        assigned = false;
                    }
                    else
                    {
                        segment.byteRangeOffset = m_previousRangeEnd;
                        segment.hasByteRangeOffset = true;
                    }
                }
                if (assigned && segment.byteRangeLength > (std::numeric_limits<std::size_t>::max)() - segment.byteRangeOffset)
                {
                    assigned = false;
                }
                if (assigned)
                {
                    m_previousRangeUri = resolvedUri;
                    m_previousRangeEnd = segment.byteRangeOffset + segment.byteRangeLength;
                    m_hasPreviousRange = true;
                }
            }
            else
            {
                m_hasPreviousRange = false;
                m_previousRangeUri.clear();
            }
        }
        m_valid = assigned;
        m_state = State::None;
        return;
    }

    const auto tag = tagOf(line);
    if (tag == ext_x_media)
    {
        MediaInfo media;
        media.baseUri = m_baseUri;
        media.originalUri = m_originUri;
        m_valid = media.parseContent(line);
        if (m_valid)
        {
            m_master.mediaInfos.emplace_back(std::move(media));
        }
    }
    else if (tag == ext_x_stream_inf)
    {
        if (m_state != State::None)
        {
            m_valid = false;
            return;
        }
        StreamInfo stream;
        stream.baseUri = m_baseUri;
        stream.originalUri = m_originUri;
        m_valid = stream.parseContent(line);
        if (m_valid)
        {
            m_master.streams.emplace_back(std::move(stream));
            m_state = State::WaitingForStreamUri;
        }
    }
    else if (tag == extinf)
    {
        if (m_state != State::None || m_playlist.hasEndList ||
            m_playlist.segments.size() > (std::numeric_limits<std::uint64_t>::max)() - m_playlist.mediaSequence)
        {
            m_valid = false;
            return;
        }
        MediaSegment segment;
        segment.originalUri = m_originUri;
        segment.baseUri = m_baseUri;
        segment.mediaSequence = m_playlist.mediaSequence + m_playlist.segments.size();
        segment.keyInfo = m_activeKey;
        if (m_playlist.hasMap)
        {
            segment.mediaMapIndex = m_playlist.mediaMaps.size() - 1;
        }
        m_valid = segment.parseContent(line);
        if (m_valid)
        {
            m_playlist.segments.emplace_back(std::move(segment));
            m_state = State::WaitingForSegmentUri;
        }
    }
    else if (tag == ext_x_key)
    {
        KeyInfo key;
        key.originalUri = m_originUri;
        key.baseUri = m_baseUri;
        m_valid = key.parseContent(line) && (key.method == KeyInfo::None || !key.getUri().empty());
        if (m_valid)
        {
            m_activeKey = key;
            m_playlist.keyInfo = std::move(key);
        }
    }
    else if (tag == ext_x_targetduration)
    {
        m_valid = util::parseNumber(line.substr(line.find(':') + 1), m_playlist.targetDuration) && m_playlist.targetDuration > 0;
    }
    else if (tag == ext_x_media_sequence)
    {
        m_valid = m_playlist.segments.empty() && util::parseNumber(line.substr(line.find(':') + 1), m_playlist.mediaSequence);
    }
    else if (tag == ext_x_byterange)
    {
        if (m_state != State::WaitingForSegmentUri || m_playlist.segments.empty())
        {
            m_valid = false;
            return;
        }
        auto& segment = m_playlist.segments.back();
        const auto range = line.substr(line.find(':') + 1);
        const auto at = range.find('@');
        m_valid = segment.byteRangeLength == 0 && util::parseNumber(range.substr(0, at), segment.byteRangeLength) && segment.byteRangeLength != 0;
        if (m_valid && at != std::string_view::npos)
        {
            m_valid = util::parseNumber(range.substr(at + 1), segment.byteRangeOffset);
            segment.hasByteRangeOffset = m_valid;
        }
        m_playlist.hasByteRange = m_valid;
    }
    else if (tag == ext_x_map)
    {
        MediaMap map;
        map.originalUri = m_originUri;
        map.baseUri = m_baseUri;
        map.keyInfo = m_activeKey;
        m_valid = map.parseContent(line) && !(map.keyInfo.method == KeyInfo::Aes128 && map.keyInfo.iv.empty());
        if (m_valid)
        {
            m_playlist.mediaMap = map;
            m_playlist.mediaMaps.emplace_back(std::move(map));
            m_playlist.hasMap = true;
            if (m_state == State::WaitingForSegmentUri && !m_playlist.segments.empty())
            {
                m_playlist.segments.back().mediaMapIndex = m_playlist.mediaMaps.size() - 1;
            }
        }
    }
    else if (tag == ext_x_discontinuity)
    {
        m_playlist.hasDiscontinuity = true;
    }
    else if (tag == ext_x_gap)
    {
        m_playlist.hasGap = true;
    }
    else if (tag == ext_i_frames_only)
    {
        m_playlist.hasIFramesOnly = true;
    }
    else if (tag == ext_x_endlist)
    {
        m_valid = m_state == State::None;
        m_playlist.hasEndList = true;
    }
    else
    {
        m_unparsedTags.emplace_back(line);
    }
}

const M3U8Master& HLSParser::master() const
{
    return m_master;
}

bool HLSParser::isMaster() const
{
    return !m_master.streams.empty();
}

const M3U8Playlist& HLSParser::playlist() const
{
    return m_playlist;
}

const std::vector<std::string>& HLSParser::unparsedTags() const
{
    return m_unparsedTags;
}

void HLSParser::setOriginUri(std::string uri)
{
    m_originUri = std::move(uri);
    const auto query = m_originUri.find_first_of("?#");
    const auto pathEnd = query == std::string::npos ? m_originUri.size() : query;
    const auto slash = m_originUri.rfind('/', pathEnd);
    m_baseUri = slash == std::string::npos ? m_originUri : m_originUri.substr(0, slash + 1);
}

const std::string& HLSParser::originUri() const
{
    return m_originUri;
}

}  // namespace hlsapi
