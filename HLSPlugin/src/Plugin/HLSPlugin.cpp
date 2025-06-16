#include "HLSPlugin.h"
#include "HLSApi/HLSClient.h"
#include "HLSApi/HLSConstants.h"
#include "HLSApi/HLSParser.h"
#include "HLSResource.h"
#include "HLSDownloader.h"
#include "Convert.h"
#include "HLSPluginMessage.h"
#include "HLSApi/HLSLog.h"
#include "FFmpeg/FFmpegHelper.h"

#include <Util/UrlProccess.h>
#include <Util/TimerUtil.h>
#include <Util/LocaleHelper.h>

constexpr char temp[] = "temp";
constexpr char cover[] = "cover";

bool cutImage(const hlsapi::M3U8Playlist& plyalist, std::string& imagePath)
{
    if (plyalist.segments.empty())
    {
        HLS_LOG_ERROR("segments is empty, image path: {}", imagePath);
        return false;
    }

    const auto& segment = plyalist.segments[plyalist.segments.size() / 2];
    auto& client = hlsapi::HLSClient::globalClient();

    std::string tsFileName = HLSPlugin::getDir() + temp + "/" + std::to_string(time(nullptr)) + "_" + std::to_string(std::rand() % 1000) + ".ts";
    if (!std::filesystem::exists(std::filesystem::path(tsFileName).parent_path()))
    {
        std::filesystem::create_directories(std::filesystem::path(tsFileName).parent_path());
    }
    client.downloadTsToFiles(segment.getUri(), util::localeToUtf8(tsFileName), plyalist.keyInfo);

    std::vector<std::string> ffmpegArgs;

    ffmpegArgs.push_back("-i");
    ffmpegArgs.push_back(util::localeToUtf8(tsFileName));
    ffmpegArgs.push_back("-ss");
    ffmpegArgs.push_back("00:00:00");
    ffmpegArgs.push_back("-vframes");
    ffmpegArgs.push_back("1");
    ffmpegArgs.push_back(util::localeToUtf8(imagePath));
    ffmpegArgs.push_back("-y");

    ffmpeg::FFmpegHelper::globalInstance().startFFmpeg(
        ffmpegArgs, []() {},
        [tsFileName]() {
            if (std::filesystem::exists(tsFileName))
            {
                std::filesystem::remove(tsFileName);
            }
        });

    return true;
}

std::string_view getUrlLast(std::string_view url)
{
    if (auto pos = url.find('?'); pos != std::string_view::npos)
    {
        url.remove_suffix(url.size() - pos);
    }

    auto last_slash = url.rfind('/');
    if (last_slash != std::string_view::npos)
    {
        return url.substr(last_slash + 1);
    }

    return url;
}

PluginMessage HLSPlugin::m_pluginMessage = {
    hlsplugin::pluginID, hlsplugin::name, hlsplugin::version, hlsplugin::description, hlsplugin::domain,
};

std::string HLSPlugin::m_dir;

const PluginMessage& HLSPlugin::pluginMessage() const
{
    return m_pluginMessage;
}

const std::vector<uint8_t>& HLSPlugin::websiteIcon()
{
    return qrc_website_icon;
}

bool HLSPlugin::canParseUrl(const std::string& url)
{
    std::string m3u8Content = hlsapi::HLSClient::globalClient().downloadM3u8(url);
    m3u8Content = hlsapi::trim(m3u8Content);
    return m3u8Content.starts_with(hlsapi::ext_m3u);
}

adapter::VideoView HLSPlugin::getVideoView(const std::string& url)
{
    HLS_LOG_INFO("getVideoView url: {}", url);
    std::vector<adapter::BaseVideoView> views;
    std::string m3u8Content = hlsapi::HLSClient::globalClient().downloadM3u8(url);
    m3u8Content = hlsapi::trim(m3u8Content);
    hlsapi::HLSParser parser;
    parser.setOriginUri(url);
    parser.parse(m3u8Content);

    auto& mediaInfos = parser.master().mediaInfos;

    hlsapi::M3U8Playlist plyalist;
    if (parser.playlist().segments.empty())
    {
        auto& streamInfo = parser.master().streams;
        for (const auto& stream : streamInfo)
        {
            if (stream.isVideoStream())
            {
                auto mediaInfoM3u8 = hlsapi::HLSClient::globalClient().downloadM3u8(stream.getUri());
                mediaInfoM3u8 = hlsapi::trim(mediaInfoM3u8);
                hlsapi::HLSParser mediaInfoParser;
                mediaInfoParser.setOriginUri(stream.getUri());
                mediaInfoParser.parse(mediaInfoM3u8);
                plyalist = mediaInfoParser.playlist();
                break;
            }
        }
    }
    else
    {
        plyalist = parser.playlist();
    }

    std::string path = m_dir + cover + "/" + "HLS_" + std::to_string(time(nullptr)) + "_" + std::to_string(std::rand() % 1000000) + ".jpg";
    if (!std::filesystem::exists(std::filesystem::path(path).parent_path()))
    {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }
    cutImage(plyalist, path);

    adapter::BaseVideoView baseView;
    baseView.Identifier = url;
    if (!plyalist.segments.empty() && !plyalist.segments[0].title.empty())
    {
        baseView.Title = plyalist.segments[0].title;
    }
    else
    {
        baseView.Title = getUrlLast(url);
    }
    baseView.Cover = util::localeToUtf8(path);
    baseView.Duration = formatDuration(plyalist.durationAll());
    std::string Description;
    baseView.pluginType = pluginMessage().pluginId;
    views.push_back(baseView);
    return views;
}

std::shared_ptr<download::FileDownloader> HLSPlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    auto copyedVideoInfo = videoInfo;
    copyedVideoInfo.downloadConfig = std::make_shared<DownloadConfig>(*videoInfo.downloadConfig);

    auto hlsClient = std::make_shared<hlsapi::HLSClient>();
    std::string url = copyedVideoInfo.videoView->Identifier;
    std::string m3u8Content = hlsClient->downloadM3u8(url);
    m3u8Content = hlsapi::trim(m3u8Content);
    hlsapi::HLSParser parser;
    parser.setOriginUri(url);
    parser.parse(m3u8Content);

    std::unordered_map<hlsapi::MediaInfo::MediaType, hlsapi::M3U8Playlist> mediaInfoMap;
    if (parser.playlist().segments.empty())
    {
        auto& streamInfo = parser.master().streams;
        hlsapi::StreamInfo defaultStreamInfo;
        for (const auto& stream : streamInfo)
        {
            if (stream.bandwidth > defaultStreamInfo.bandwidth)
            {
                defaultStreamInfo = stream;
            }
        }

        {
            auto mediaInfoM3u8 = hlsClient->downloadM3u8(defaultStreamInfo.getUri());
            mediaInfoM3u8 = hlsapi::trim(mediaInfoM3u8);
            hlsapi::HLSParser mediaInfoParser;
            mediaInfoParser.setOriginUri(defaultStreamInfo.getUri());
            mediaInfoParser.parse(mediaInfoM3u8);
            mediaInfoMap[hlsapi::MediaInfo::Video] = mediaInfoParser.playlist();
        }

        bool hasAudio = false;
        bool hasSubtitle = false;
        for (const auto& mediaInfo : parser.master().mediaInfos)
        {
            if (mediaInfo.type == hlsapi::MediaInfo::Video)
            {
                continue;
            }
            else if (mediaInfo.type == hlsapi::MediaInfo::Audio)
            {
                if (hasAudio)
                {
                    continue;
                }

                auto mediaInfoM3u8 = hlsapi::HLSClient::globalClient().downloadM3u8(mediaInfo.getUri());
                mediaInfoM3u8 = hlsapi::trim(mediaInfoM3u8);
                hlsapi::HLSParser mediaInfoParser;
                mediaInfoParser.setOriginUri(mediaInfo.getUri());
                mediaInfoParser.parse(mediaInfoM3u8);
                mediaInfoMap[mediaInfo.type] = mediaInfoParser.playlist();
                hasAudio = true;
            }
            else if (mediaInfo.type == hlsapi::MediaInfo::Subtitle || mediaInfo.type == hlsapi::MediaInfo::CloseCaption)
            {
                if (hasSubtitle)
                {
                    continue;
                }

                auto mediaInfoM3u8 = hlsClient->downloadM3u8(mediaInfo.getUri());
                mediaInfoM3u8 = hlsapi::trim(mediaInfoM3u8);
                hlsapi::HLSParser mediaInfoParser;
                mediaInfoParser.setOriginUri(mediaInfo.getUri());
                mediaInfoParser.parse(mediaInfoM3u8);
                mediaInfoMap[mediaInfo.type] = mediaInfoParser.playlist();
                hasSubtitle = true;
            }
        }
    }
    else
    {
        mediaInfoMap[hlsapi::MediaInfo::Video] = parser.playlist();
    }

    auto hlsDownlaoder = std::shared_ptr<download::HLSDownloader>(new download::HLSDownloader(hlsClient, mediaInfoMap), download::freeDownload);
    hlsDownlaoder->setPath(copyedVideoInfo.downloadConfig->downloadDir);
    std::string fileName = copyedVideoInfo.fileName();
    if (fileName.empty())
    {
        fileName = std::string(getUrlLast(copyedVideoInfo.videoView->Identifier)) + "_" + std::to_string(std::rand() % 10000);
    }
    hlsDownlaoder->setFilename(fileName + ".mp4");
    return hlsDownlaoder;
}

LoginProxy HLSPlugin::loginer()
{
    return LoginProxy(m_login);
}

void HLSPlugin::setDir(std::string dir)
{
    m_dir = dir;
}

const std::string& HLSPlugin::getDir()
{
    return m_dir;
}
