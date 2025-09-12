#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <string_view>

namespace hlsapi
{

std::unordered_map<std::string_view, std::string_view> parseAttributes(std::string_view line);
bool attrBool(std::string_view attr);
std::string_view trim(std::string_view sv);

struct PendingInfo
{
    std::string uri;
    std::string baseUri;
    std::string originalUri;

    std::string getUri() const;
};

struct StreamInfo : PendingInfo
{
    int bandwidth{};
    double score{};
    std::string codecs;
    std::string supplementalCodecs;
    std::string resolution;
    int frameRate{};
    std::string hdcpLevel;
    std::string allowedCPC;
    std::string videoRange;
    std::string reqVideoLayout;
    std::string stableVariantId;
    std::string video;
    std::string audio;
    std::string subtitles;
    std::string closedCaptions;
    std::string pathwayId;
    int programId{};

    bool isVideoStream() const;

    bool parseContent(std::string_view content);
};

struct MediaInfo : PendingInfo
{
    enum MediaType
    {
        Video,
        Audio,
        Subtitle,
        CloseCaption,
    };

    static std::string mediaTypeToString(MediaType type);
    static MediaType stringToMediaType(std::string type);

    MediaType type{};
    std::string strType;
    std::string groupId;
    std::string language;
    std::string assoc_language;
    std::string name;
    std::string stableRenditionId;
    bool strDefault{};
    bool autoSelect{};
    bool forced{};
    std::string instream_id;
    int bitDepth{};
    int sampleRate{};
    std::string characteristics;
    std::string channels;

    bool parseContent(std::string_view content);
};

struct M3U8Master
{
    std::vector<StreamInfo> streams;
    std::vector<MediaInfo> mediaInfos;
};

struct MediaSegment : PendingInfo
{
    double duration{};
    std::string title;
    std::size_t byteRangeLength{};
    std::size_t byteRangeOffset{};
    bool parseContent(std::string_view content);
};

struct KeyInfo : PendingInfo
{
    enum KeyMethod
    {
        None,
        Aes128,
        SampleAes,
        SampleAesCtr,
    };

    static KeyMethod stringToKeyMethod(std::string method);
    static std::string keyMethodToString(KeyMethod method);

    KeyMethod method{None};
    std::string strMethod;
    std::string iv;
    std::string keyFormat;
    std::string keyFormatVersions;
    std::string key;

    bool parseContent(std::string_view content);
};

struct MediaMap : PendingInfo
{
    std::size_t byteRangeLength{};
    std::size_t byteRangeOffset{};

    bool parseContent(std::string_view content);
};

struct M3U8Playlist
{
    int mediaSequence = 0;
    double targetDuration = 0;
    KeyInfo keyInfo;
    std::vector<MediaSegment> segments;

    double durationAll() const;
};

}  // namespace hlsapi
