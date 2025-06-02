#include "HLSParser.h"

#include <sstream>

#include "HLSConstants.h"
namespace hlsapi
{

bool HLSParser::parse(std::string_view content)
{
    size_t start = 0;

    while (start < content.size())
    {
        size_t line_end = start;
        while (line_end < content.size() && content[line_end] != '\n' && content[line_end] != '\r')
        {
            ++line_end;
        }

        std::string_view line = content.substr(start, line_end - start);
        line = trim(line);

        if (!line.empty())
        {
            parseLine(line);
        }

        if (line_end < content.size())
        {
            if (content[line_end] == '\r' && line_end + 1 < content.size() && content[line_end + 1] == '\n')
            {
                start = line_end + 2;
            }
            else
            {
                start = line_end + 1;
            }
        }
        else
        {
            start = content.size();
        }
    }

    return true;
}

void HLSParser::parseLine(std::string_view line)
{
    if (line.empty() || line[0] != '#')
    {
        if (m_state == State::WaitingForStreamUri)
        {
            if (m_pendingInfo != nullptr)
            {
                m_pendingInfo->uri = std::string(line);
            }

            m_state = State::None;
        }
    }

    if (line.starts_with(ext_x_media))
    {
        MediaInfo media;
        media.baseUri = m_baseUri;
        media.originalUri = m_originUri;
        media.parseContent(line);
        m_master.mediaInfos.emplace_back(std::move(media));
    }
    else if (line.starts_with(ext_x_stream_inf))
    {
        StreamInfo stream;
        stream.baseUri = m_baseUri;
        stream.originalUri = m_originUri;
        stream.parseContent(line);
        m_master.streams.emplace_back(std::move(stream));
        m_pendingInfo = &m_master.streams.back();
        m_state = State::WaitingForStreamUri;
    }
    else if (line.starts_with(extinf))
    {
        MediaSegment segment;
        segment.parseContent(line);
        segment.originalUri = m_originUri;
        segment.baseUri = m_baseUri;
        m_playlist.segments.emplace_back(std::move(segment));
        m_pendingInfo = &m_playlist.segments.back();
        m_pendingMediaSegment = &m_playlist.segments.back();
        m_state = State::WaitingForStreamUri;
    }
    else if (line.starts_with(ext_x_key))
    {
        KeyInfo key;
        key.parseContent(line);
        m_playlist.keyInfo = std::move(key);
    }
    else if (line.starts_with(ext_x_byterange))
    {
        if (m_pendingMediaSegment == nullptr)
        {
            return;
        }

        std::string value = std::string(line.substr(line.find(':') + 1));
        value = trim(value);
        size_t at = value.find('@');
        size_t length = std::stoul(value.substr(0, at));
        m_pendingMediaSegment->byteRangeLength = length;
        if (at != std::string::npos)
        {
            size_t offset = std::stoul(value.substr(at + 1));
            m_pendingMediaSegment->byteRangeOffset = offset;
        }
    }
    else if (line.starts_with(ext_x_map))
    {
        MediaMap mediaMap;
        mediaMap.baseUri = m_baseUri;
        mediaMap.originalUri = m_originUri;
        mediaMap.parseContent(line);
    }
    else if (line.starts_with(ext_x_targetduration))
    {
        m_targetDuration = std::atof(line.substr(ext_x_targetduration.size()).data() + 1);
    }
    else if (line.starts_with(ext_x_media_sequence))
    {
        m_mediaSequence = std::atoi(line.substr(ext_x_media_sequence.size()).data() + 1);
    }
}

const M3U8Master& HLSParser::master() const
{
    return m_master;
}

bool HLSParser::isMaster() const
{
    return m_playlist.segments.empty();
}

const M3U8Playlist& HLSParser::playlist() const
{
    return m_playlist;
}

void HLSParser::setOriginUri(std::string uri)
{
    m_originUri = std::move(uri);
    m_baseUri = m_originUri.substr(0, m_originUri.find_last_of('/') + 1);
}

const std::string& HLSParser::originUri() const
{
    return m_originUri;
}

}  // namespace hlsapi
