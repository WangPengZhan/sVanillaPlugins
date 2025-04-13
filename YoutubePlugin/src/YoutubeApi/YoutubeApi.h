#pragma once

#include <nlohmann/json.hpp>
#include <list>
#include <string>
#include <vector>

namespace youtubeapi
{
// https://github.com/Tyrrrz/YoutubeExplode

struct Pair
{
    std::string key;
    std::string value;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Pair, key, value)
};

// ServiceTrackingParam
struct ServiceTrackingParam
{
    std::string service;
    std::vector<Pair> params;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ServiceTrackingParam, service, params)
};

struct MainAppWebResponseContext
{
    std::string datasyncId;
    bool loggedOut;
    std::string trackingParam;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MainAppWebResponseContext, datasyncId, loggedOut, trackingParam)
};

struct WebResponseContextExtensionData
{
    bool hasDecorated;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WebResponseContextExtensionData, hasDecorated)
};

struct ResponseContext
{
    std::string visitorData;
    std::vector<ServiceTrackingParam> serviceTrackingParams;
    int maxAgeSeconds;
    MainAppWebResponseContext mainAppWebResponseContext;
    WebResponseContextExtensionData webResponseContextExtensionData;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ResponseContext, visitorData, serviceTrackingParams, maxAgeSeconds, mainAppWebResponseContext,
                                                webResponseContextExtensionData)
};

struct ThumbnailData
{
    std::string url;
    int width;
    int height;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ThumbnailData, url, width, height)
};

struct Thumbnail
{
    std::vector<ThumbnailData> thumbnails;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Thumbnail, thumbnails)
};

struct UrlEndpoint
{
    std::string url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UrlEndpoint, url)
};

struct NavigationEndpoint
{
    std::string clickTrackingParams;
    UrlEndpoint urlEndpoint;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NavigationEndpoint, clickTrackingParams, urlEndpoint)
};

struct Run
{
    std::string text;
    NavigationEndpoint navigationEndpoint;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Run, text, navigationEndpoint)
};

struct ButtonRenderer
{
    std::string style;
    std::string size;
    bool isDisabled;
    struct SimpleText
    {
        std::string simpleText;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SimpleText, simpleText)
    } text;
    NavigationEndpoint navigationEndpoint;
    std::string trackingParams;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ButtonRenderer, style, size, isDisabled, text, navigationEndpoint, trackingParams)
};

struct PlayerErrorMessageRenderer
{
    std::vector<Run> subreason;
    std::vector<Run> reason;
    ButtonRenderer proceedButton;
    Thumbnail thumbnail;
    struct Icon
    {
        std::string iconType;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Icon, iconType)
    } icon;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayerErrorMessageRenderer, subreason, reason, proceedButton, thumbnail, icon)
};

struct PlayabilityStatus
{
    std::string status;
    bool playableInEmbed{};
    std::string reason;
    PlayerErrorMessageRenderer errorScreen;
    struct Skip
    {
        struct PlayabilityErrorSkipConfig
        {
            bool skipOnPlayabilityError;

            NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayabilityErrorSkipConfig, skipOnPlayabilityError)
        };
        PlayabilityErrorSkipConfig playabilityErrorSkipConfig;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Skip, playabilityErrorSkipConfig)
    } skip;

    struct PictureInPicture
    {
        struct PictureInPictureRenderer
        {
            bool offByDefault;

            NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PictureInPictureRenderer, offByDefault)
        };
        PictureInPictureRenderer pictureInPictureRenderer;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PictureInPicture, pictureInPictureRenderer)
    } pictureInPicture;

    std::string contextParams;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayabilityStatus, status, playableInEmbed, reason, errorScreen, skip, pictureInPicture, contextParams)
};

struct Format
{
    int itag;
    std::string url;
    std::string mimeType;
    int bitrate;
    int width;
    int height;
    std::string lastModified;
    std::string quality;
    int fps;
    std::string qualityLabel;
    std::string projectionType;
    std::string audioQuality;
    std::string approxDurationMs;
    int audioSampleRate;
    int audioChannels;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Format, itag, url, mimeType, bitrate, width, height, lastModified, quality, fps, qualityLabel, projectionType,
                                                audioQuality, approxDurationMs, audioSampleRate, audioChannels)
};

struct ColorInfo
{
    std::string primaries;
    std::string transferCharacteristics;
    std::string matrixCoefficients;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ColorInfo, primaries, transferCharacteristics, matrixCoefficients)
};

struct Range
{
    std::string start;
    std::string end;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Range, start, end)
};

struct AdaptiveFormat
{
    int itag{};
    std::string url;
    std::string mimeType;
    int bitrate{};
    int width{};
    int height{};
    Range initRange;
    Range indexRange;
    std::string lastModified;
    std::string contentLength;
    std::string quality;
    int fps{};
    std::string qualityLabel;
    std::string projectionType;
    int averageBitrate{};
    ColorInfo colorInfo;
    std::string approxDurationMs;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AdaptiveFormat, itag, url, mimeType, bitrate, width, height, initRange, indexRange, lastModified, contentLength,
                                                quality, fps, qualityLabel, projectionType, averageBitrate, colorInfo, approxDurationMs)
};

struct StreamingData
{
    std::string expiresInSeconds;
    std::vector<Format> formats;
    std::vector<AdaptiveFormat> adaptiveFormats;
    std::string hlsManifestUrl;
    double aspectRatio{};
    std::string serverAbrStreamingUrl;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(StreamingData, expiresInSeconds, formats, adaptiveFormats, hlsManifestUrl, aspectRatio, serverAbrStreamingUrl)
};

struct UpdateEntityCommand
{
    std::string identifier;
    std::string transform;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UpdateEntityCommand, identifier, transform)
};

struct ElementsCommand
{
    UpdateEntityCommand updateEntityCommand;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ElementsCommand, updateEntityCommand)
};

struct OnResponseReceivedEndpoint
{
    std::string clickTrackingParams;
    ElementsCommand elementsCommand;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(OnResponseReceivedEndpoint, clickTrackingParams, elementsCommand)
};

struct OfflineabilityEntity
{
    std::string key;
    std::string addToOfflineButtonState;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(OfflineabilityEntity, key, addToOfflineButtonState)
};

struct Payload
{
    OfflineabilityEntity offlineabilityEntity;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Payload, offlineabilityEntity)
};

struct Mutation
{
    std::string entityKey;
    std::string type;
    Payload payload;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Mutation, entityKey, type, payload)
};

struct Timestamp
{
    std::string seconds;
    int nanos;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Timestamp, seconds, nanos)
};

struct EntityBatchUpdate
{
    std::vector<Mutation> mutations;
    Timestamp timestamp;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(EntityBatchUpdate, mutations, timestamp)
};

struct FrameworkUpdates
{
    EntityBatchUpdate entityBatchUpdate;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FrameworkUpdates, entityBatchUpdate)
};

struct VideoDetails
{
    std::string videoId;
    std::string title;
    std::string lengthSeconds;
    std::string channelId;
    bool isOwnerViewing;
    std::string shortDescription;
    bool isCrawlable;
    Thumbnail thumbnail;
    bool allowRatings;
    std::string viewCount;
    std::string author;
    bool isPrivate;
    bool isUnpluggedCorpus;
    bool isLiveContent;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoDetails, videoId, title, lengthSeconds, channelId, isOwnerViewing, shortDescription, isCrawlable,
                                                thumbnail, allowRatings, viewCount, author, isPrivate, isUnpluggedCorpus, isLiveContent)
};

struct PlayerMicroformatRenderer
{
    std::string publishDate;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayerMicroformatRenderer, publishDate)
};

struct Microformat
{
    PlayerMicroformatRenderer playerMicroformatRenderer;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Microformat, playerMicroformatRenderer)
};

struct MainResponse
{
    ResponseContext responseContext;
    PlayabilityStatus playabilityStatus;
    std::string trackingParams;
    std::vector<OnResponseReceivedEndpoint> onResponseReceivedEndpoints;
    std::string adBreakHeartbeatParams;
    FrameworkUpdates frameworkUpdates;
    VideoDetails videoDetails;
    Microformat microformat;
    StreamingData streamingData;

    bool isAvailable() const
    {
        std::string status = playabilityStatus.status;
        std::transform(status.begin(), status.end(), status.begin(), ::tolower);
        return status == "error";
    }

    bool isPlayable() const
    {
        std::string status = playabilityStatus.status;
        std::transform(status.begin(), status.end(), status.begin(), ::tolower);
        return status == "ok";
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MainResponse, responseContext, playabilityStatus, trackingParams, onResponseReceivedEndpoints,
                                                adBreakHeartbeatParams, frameworkUpdates, videoDetails, microformat, streamingData)
};

}  // namespace youtubeapi
