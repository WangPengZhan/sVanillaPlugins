#include "HLSApi.h"

#include <algorithm>
#include <curl/curl.h>

#include "HLSConstants.h"
#include "Util/NumberParse.h"

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
    while (!line.empty())
    {
        const auto eq = line.find('=');
        if (eq == std::string_view::npos)
        {
            return {};
        }

        const auto key = trim(line.substr(0, eq));
        if (key.empty() || key.find(',') != std::string_view::npos)
        {
            return {};
        }
        if (attrs.contains(key))
        {
            return {};
        }

        line = trim(line.substr(eq + 1));
        if (line.empty())
        {
            return {};
        }

        std::string_view val;
        if (line.front() == '"')
        {
            const auto end = line.find('"', 1);
            if (end == std::string_view::npos)
            {
                return {};
            }

            val = line.substr(1, end - 1);
            line = trim(line.substr(end + 1));
            if (!line.empty() && line.front() != ',')
            {
                return {};
            }
        }
        else
        {
            const auto end = line.find(',');
            val = trim(line.substr(0, end));
            if (val.empty())
            {
                return {};
            }

            line = (end == std::string_view::npos) ? std::string_view{} : line.substr(end);
        }
        attrs.emplace(key, val);
        if (!line.empty())
        {
            line = trim(line.substr(1));
            if (line.empty())
            {
                return {};
            }
        }
    }

    return attrs;
}

std::string PendingInfo::getUri() const
{
    if (uri.empty())
    {
        return {};
    }
    CURLU* handle = curl_url();
    if (!handle)
    {
        return {};
    }
    const std::string base = originalUri.empty() ? baseUri : originalUri;
    if ((!base.empty() && curl_url_set(handle, CURLUPART_URL, base.c_str(), 0) != CURLUE_OK) ||
        curl_url_set(handle, CURLUPART_URL, uri.c_str(), 0) != CURLUE_OK)
    {
        curl_url_cleanup(handle);
        return {};
    }
    char* scheme = nullptr;
    char* resolved = nullptr;
    std::string output;
    if (curl_url_get(handle, CURLUPART_SCHEME, &scheme, 0) == CURLUE_OK && scheme &&
        (std::string_view(scheme) == "http" || std::string_view(scheme) == "https") && curl_url_get(handle, CURLUPART_URL, &resolved, 0) == CURLUE_OK &&
        resolved)
    {
        output = resolved;
    }
    curl_free(scheme);
    curl_free(resolved);
    curl_url_cleanup(handle);
    return output;
}

bool StreamInfo::isVideoStream() const
{
    if (codecs.find("avc1") != std::string::npos || codecs.find("hvc1") != std::string::npos || codecs.find("hev1") != std::string::npos ||
        codecs.find("vp09") != std::string::npos || codecs.find("av01") != std::string::npos)
    {
        return true;
    }
    if (!resolution.empty())
    {
        return true;
    }
    if (!video.empty())
    {
        return true;
    }
    return false;
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
    if (attrs.empty())
    {
        return false;
    }

    const auto bandwidthIt = attrs.contains("BANDWIDTH") ? attrs.find("BANDWIDTH") : attrs.find("AVERAGE-BANDWIDTH");
    if (bandwidthIt == attrs.end() || !util::parseNumber(bandwidthIt->second, bandwidth))
    {
        return false;
    }

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
        if (!util::parseNumber(attrs.at("FRAME-RATE"), frameRate))
        {
            return false;
        }
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

    return true;
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

    if (attrs.empty() || !attrs.contains("TYPE") || !attrs.contains("GROUP-ID") || !attrs.contains("NAME"))
    {
        return false;
    }
    if (attrs.at("TYPE") != media_type_video && attrs.at("TYPE") != media_type_audio && attrs.at("TYPE") != media_type_subtitle &&
        attrs.at("TYPE") != media_type_closed_caption)
    {
        return false;
    }

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
        if (!util::parseNumber(attrs.at("BITRATE"), bitDepth))
        {
            return false;
        }
    }
    if (attrs.find("SAMPLE-RATE") != attrs.end())
    {
        if (!util::parseNumber(attrs.at("SAMPLE-RATE"), sampleRate))
        {
            return false;
        }
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

    const auto pos = content.find(':');
    const auto endPos = content.find(',', pos == std::string_view::npos ? 0 : pos + 1);
    if (pos == std::string_view::npos || endPos == std::string_view::npos)
    {
        return false;
    }
    const auto strDuration = trim(content.substr(pos + 1, endPos - pos - 1));
    if (!util::parseNumber(strDuration, duration))
    {
        return false;
    }
    title = content.substr(endPos + 1);
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
    if (attrs.empty() || !attrs.contains("METHOD"))
    {
        return false;
    }

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

    if (method != KeyMethod::None && uri.empty())
    {
        return false;
    }

    return true;
}

bool MediaMap::parseContent(std::string_view content)
{
    if (!content.starts_with(ext_x_map + ":"))
    {
        return false;
    }

    content = content.substr(content.find(':') + 1);
    content = trim(content);
    const auto attrs = parseAttributes(content);
    const auto uriAttribute = attrs.find("URI");
    if (uriAttribute == attrs.end() || uriAttribute->second.empty())
    {
        return false;
    }

    uri = uriAttribute->second;
    hasByteRange = false;
    hasByteRangeOffset = false;
    byteRangeLength = 0;
    byteRangeOffset = 0;

    const auto range = attrs.find("BYTERANGE");
    if (range == attrs.end())
    {
        return true;
    }

    const auto value = range->second;
    const size_t at = value.find('@');
    if (!util::parseNumber(value.substr(0, at), byteRangeLength))
    {
        return false;
    }
    if (byteRangeLength == 0)
    {
        return false;
    }
    if (at != std::string::npos)
    {
        if (!util::parseNumber(value.substr(at + 1), byteRangeOffset))
        {
            return false;
        }
        if (byteRangeLength > (std::numeric_limits<std::size_t>::max)() - byteRangeOffset)
        {
            return false;
        }
        hasByteRangeOffset = true;
    }
    hasByteRange = true;
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

bool M3U8Playlist::isSupportedVod() const
{
    if (!hasEndList || hasGap || hasIFramesOnly)
    {
        return false;
    }
    const auto supportedKey = [](const KeyInfo& key) {
        return key.method == KeyInfo::None ||
               (key.method == KeyInfo::Aes128 && (key.keyFormat.empty() || key.keyFormat == "identity"));
    };
    for (const auto& segment : segments)
    {
        if (!supportedKey(segment.keyInfo) || (segment.byteRangeLength != 0 && !segment.hasByteRangeOffset))
        {
            return false;
        }
    }

    if (!hasMap)
    {
        return std::none_of(segments.begin(), segments.end(), [](const MediaSegment& segment) { return segment.mediaMapIndex.has_value(); });
    }
    if (mediaMaps.empty() || std::any_of(mediaMaps.begin(), mediaMaps.end(), [&](const MediaMap& map) {
            return map.getUri().empty() || !supportedKey(map.keyInfo) || (map.keyInfo.method == KeyInfo::Aes128 && map.keyInfo.iv.empty()) ||
                   (map.hasByteRange && !map.hasByteRangeOffset);
        }))
    {
        return false;
    }
    return std::all_of(segments.begin(), segments.end(), [&](const MediaSegment& segment) {
        return segment.mediaMapIndex.has_value() && *segment.mediaMapIndex < mediaMaps.size();
    });
}

}  // namespace hlsapi
