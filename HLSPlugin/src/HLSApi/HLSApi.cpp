#include "HLSApi.h"

#include "HLSConstants.h"

namespace hlsapi
{
bool attrBool(std::string_view attr)
{
    return attr == "YES" || attr == "true" || attr == "1";
}

std::string_view trim(std::string_view sv)
{
    const auto begin = sv.find_first_not_of(" \t\r\n");
    if (begin == std::string_view::npos)
        return {};
    const auto end = sv.find_last_not_of(" \t\r\n");
    return sv.substr(begin, end - begin + 1);
}

std::unordered_map<std::string_view, std::string_view> parseAttributes(std::string_view line)
{
    std::unordered_map<std::string_view, std::string_view> attrs;
    size_t start = 0;

    while (start < line.size())
    {
        size_t eq = line.find('=', start);
        if (eq == std::string_view::npos)
        {
            break;
        }
        auto key = line.substr(start, eq - start);
        size_t val_start = eq + 1;
        std::string_view val;
        if (line[val_start] == '"')
        {
            size_t end = line.find('"', val_start + 1);
            val = line.substr(val_start + 1, end - val_start - 1);
            start = (line.find(',', end) != std::string_view::npos) ? line.find(',', end) + 1 : line.size();
        }
        else
        {
            size_t end = line.find(',', val_start);
            val = line.substr(val_start, end - val_start);
            start = (end != std::string_view::npos) ? end + 1 : line.size();
        }
        attrs[key] = val;
    }

    return attrs;
}

std::string PendingInfo::getUri() const
{
    if (uri.starts_with("http://") || uri.starts_with("https://"))
    {
        return uri;
    }

    return baseUri + uri;
}

bool StreamInfo::parseContent(std::string_view content)
{
    if (!content.starts_with(ext_x_stream_inf))
    {
        return false;
    }

    content = content.substr(content.find(':') + 1);
    content = trim(content);
    auto attrs = parseAttributes(content);
    auto bandwidthString = (attrs.find("BANDWIDTH") == attrs.end()) ? attrs["AVERAGE-BANDWIDTH"] : attrs["BANDWIDTH"];
    bandwidth = std::atoi(bandwidthString.data());

    if (attrs.find("CODECS") != attrs.end())
    {
        codecs = attrs["CODECS"];
    }

    if (attrs.find("SUPPLEMENTAL-CODECS") != attrs.end())
    {
        supplementalCodecs = attrs["SUPPLEMENTAL-CODECS"];
    }

    if (attrs.find("RESOLUTION") != attrs.end())
    {
        resolution = attrs["RESOLUTION"];
    }

    if (attrs.find("FRAME-RATE") != attrs.end())
    {
        frameRate = std::atoi(attrs["FRAME-RATE"].data());
    }

    if (attrs.find("HDCP-LEVEL") != attrs.end())
    {
        hdcpLevel = attrs["HDCP-LEVEL"];
    }

    if (attrs.find("ALLOWED-CPC") != attrs.end())
    {
        allowedCPC = attrs["ALLOWED-CPC"];
    }

    if (attrs.find("VIDEO-RANGE") != attrs.end())
    {
        videoRange = attrs["VIDEO-RANGE"];
    }

    if (attrs.find("REQ-VIDEO-LAYOUT") != attrs.end())
    {
        reqVideoLayout = attrs["REQ-VIDEO-LAYOUT"];
    }

    if (attrs.find("STABLE-VARIANT-ID") != attrs.end())
    {
        stableVariantId = attrs["STABLE-VARIANT-ID"];
    }

    if (attrs.find("VIDEO") != attrs.end())
    {
        video = attrs["VIDEO"];
    }

    if (attrs.find("AUDIO") != attrs.end())
    {
        audio = attrs["AUDIO"];
    }

    if (attrs.find("SUBTITLES") != attrs.end())
    {
        subtitles = attrs["SUBTITLES"];
    }

    if (attrs.find("CLOSED-CAPTIONS") != attrs.end())
    {
        closedCaptions = attrs["CLOSED-CAPTIONS"];
    }

    if (attrs.find("PATHWAY-ID") != attrs.end())
    {
        pathwayId = attrs["PATHWAY-ID"];
    }

    return false;
}

bool parseContent(std::string_view content);

std::string MediaInfo::mediaTypeToString(MediaType type)
{
    switch (type)
    {
    case MediaType::Video:
        return media_type_video;
    case MediaType::Audio:
        return media_type_audio;
    case MediaType::Subtitle:
        return media_type_subtitle;
    case MediaType::CloseCaption:
        return media_type_closed_caption;
    default:
        return media_type_video;
    }
}

MediaInfo::MediaType MediaInfo::stringToMediaType(std::string type)
{
    if (type == media_type_video)
    {
        return MediaType::Video;
    }
    else if (type == media_type_audio)
    {
        return MediaType::Audio;
    }
    else if (type == media_type_subtitle)
    {
        return MediaType::Subtitle;
    }
    else if (type == media_type_closed_caption)
    {
        return MediaType::CloseCaption;
    }
    else
    {
        return MediaType::Video;
    }
}

bool MediaInfo::parseContent(std::string_view content)
{
    if (!content.starts_with(ext_x_media))
    {
        return false;
    }

    content = content.substr(content.find(':') + 1);
    content = trim(content);
    auto attrs = parseAttributes(content);
    if (attrs.find("URI") != attrs.end())
    {
        uri = attrs["URI"];
    }
    if (attrs.find("TYPE") != attrs.end())
    {
        strType = attrs["TYPE"];
        type = stringToMediaType(strType);
    }
    if (attrs.find("GROUP-ID") != attrs.end())
    {
        groupId = attrs["GROUP-ID"];
    }
    if (attrs.find("LANGUAGE") != attrs.end())
    {
        language = attrs["LANGUAGE"];
    }
    if (attrs.find("ASSOC-LANGUAGE") != attrs.end())
    {
        assoc_language = attrs["ASSOC-LANGUAGE"];
    }
    if (attrs.find("NAME") != attrs.end())
    {
        name = attrs["NAME"];
    }
    if (attrs.find("STABLE-RENDITION-ID") != attrs.end())
    {
        stableRenditionId = attrs["STABLE-RENDITION-ID"];
    }
    if (attrs.find("DEFAULT") != attrs.end())
    {
        strDefault = attrBool(attrs["DEFAULT"]);
    }
    if (attrs.find("AUTOSELECT") != attrs.end())
    {
        autoSelect = attrBool(attrs["AUTOSELECT"]);
    }
    if (attrs.find("FORCED") != attrs.end())
    {
        forced = attrBool(attrs["FORCED"]);
    }
    if (attrs.find("INSTREAM-ID") != attrs.end())
    {
        instream_id = attrs["INSTREAM-ID"];
    }
    if (attrs.find("BITRATE") != attrs.end())
    {
        bitDepth = std::atoi(attrs["BITRATE"].data());
    }
    if (attrs.find("SAMPLE-RATE") != attrs.end())
    {
        sampleRate = std::atoi(attrs["SAMPLE-RATE"].data());
    }
    if (attrs.find("CHARACTERISTICS") != attrs.end())
    {
        characteristics = attrs["CHARACTERISTICS"];
    }
    if (attrs.find("CHANNELS") != attrs.end())
    {
        channels = attrs["CHANNELS"];
    }

    return true;
}

bool MediaSegment::parseContent(std::string_view content)
{
    if (!content.starts_with(extinf))
    {
        return false;
    }

    auto pos = content.find(":");
    if (pos != std::string_view::npos)
    {
        auto endPos = content.find(",");
        if (endPos != std::string_view::npos)
        {
            auto strDuration = content.substr(pos + 1, endPos - pos - 1);
            strDuration = strDuration.substr(0, strDuration.find_last_not_of(" ") + 1);
            strDuration = strDuration.substr(0, strDuration.find_last_not_of("\n") + 1);
            duration = std::atof(strDuration.data());
            if (endPos + 1 < content.size())
            {
                title = content.substr(endPos + 1, content.size() - endPos - 1);
            }
        }
    }

    return true;
}

KeyInfo::KeyMethod KeyInfo::stringToKeyMethod(std::string method)
{
    if (method == key_method_none)
    {
        return KeyMethod::None;
    }
    else if (method == key_method_aes128)
    {
        return KeyMethod::Aes128;
    }
    else if (method == key_method_sample_aes)
    {
        return KeyMethod::SampleAes;
    }
    else if (method == key_method_sample_aes_ctr)
    {
        return KeyMethod::SampleAesCtr;
    }

    return KeyMethod::None;
}

std::string KeyInfo::keyMethodToString(KeyMethod method)
{
    switch (method)
    {
    case KeyMethod::None:
        return key_method_none;
    case KeyMethod::Aes128:
        return key_method_aes128;
    case KeyMethod::SampleAes:
        return key_method_sample_aes;
    case KeyMethod::SampleAesCtr:
        return key_method_sample_aes_ctr;
    default:
        break;
    }
    return std::string();
}

bool KeyInfo::parseContent(std::string_view content)
{
    if (!content.starts_with(ext_x_key))
    {
        return false;
    }

    content = content.substr(content.find(':') + 1);
    content = trim(content);
    auto attrs = parseAttributes(content);

    if (attrs.find("METHOD") != attrs.end())
    {
        method = stringToKeyMethod(std::string(attrs["METHOD"]));
        strMethod = attrs["METHOD"];
    }
    if (attrs.find("URI") != attrs.end())
    {
        uri = attrs["URI"];
    }
    if (attrs.find("IV") != attrs.end())
    {
        iv = attrs["IV"];
    }
    if (attrs.find("KEYFORMAT") != attrs.end())
    {
        keyFormat = attrs["KEYFORMAT"];
    }
    if (attrs.find("KEYFORMATVERSIONS") != attrs.end())
    {
        keyFormatVersions = attrs["KEYFORMATVERSIONS"];
    }

    return true;
}

bool MediaMap::parseContent(std::string_view content)
{
    if (!content.starts_with(ext_x_map))
    {
        return false;
    }

    content = content.substr(content.find(':') + 1);
    content = trim(content);
    auto attrs = parseAttributes(content);

    if (attrs.find("URI") != attrs.end())
    {
        uri = attrs["URI"];
    }

    if (attrs.find("BYTERANGE") != attrs.end())
    {
        auto value = attrs["BYTERANGE"];
        value = trim(value);
        size_t at = value.find('@');
        size_t length = std::stoul(std::string(value.substr(0, at)));
        byteRangeLength = length;
        if (at != std::string::npos)
        {
            size_t offset = std::stoul(std::string(value.substr(at + 1)));
            byteRangeOffset = offset;
        }
    }

    return true;
}
double M3U8Playlist::durationAll() const
{
    double duration = 0.0;
    for (const auto& segment : segments)
    {
        duration += segment.duration;
    }
    return duration;
}
}  // namespace hlsapi
