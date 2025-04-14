#include "BiliBiliPlugin.h"
#include "BiliApi/BilibiliClient.h"
#include "BiliApi/BiliApi.h"
#include "BiliApi/BilibiliUrl.h"
#include "BiliBiliResource.h"
#include "BiliBiliDownloader.h"
#include "Convert.h"
#include "BilibiliPluginMessage.h"
#include "BiliApi/BilibiliLog.h"

#include <Util/UrlProccess.h>

PluginMessage BiliBiliPlugin::m_pluginMessage = {
    biliplugin::pluginID, biliplugin::name, biliplugin::version, biliplugin::description, biliplugin::domain,
};

std::string BiliBiliPlugin::m_dir;

const PluginMessage& BiliBiliPlugin::pluginMessage() const
{
    return m_pluginMessage;
}

const std::vector<uint8_t>& BiliBiliPlugin::websiteIcon()
{
    return qrc_website_icon;
}

bool BiliBiliPlugin::canParseUrl(const std::string& url)
{
    return isValidUrl(url);
}

adapter::VideoView BiliBiliPlugin::getVideoView(const std::string& url)
{
    BILIBILI_LOG_INFO("getVideoView url: {}", url);
    std::string id = getID(url);
    BILIBILI_LOG_INFO("id: {}", id);
    auto videoView = biliapi::BilibiliClient::globalClient().getVideoView(id);
    const auto views = convertVideoView(videoView.data);
    return views;
}

std::shared_ptr<download::FileDownloader> BiliBiliPlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    auto copyedVideoInfo = videoInfo;
    copyedVideoInfo.downloadConfig = std::make_shared<DownloadConfig>(*videoInfo.downloadConfig);

    auto& biliClient = biliapi::BilibiliClient::globalClient();
    long long qn = 64;
    if (biliClient.isLogined())
    {
        qn = 80;
    }
    if (false)
    {
        qn = 116;
    }

    long long fnval = 16;
    BILIBILI_LOG_INFO("getDownloader, guid: {}, qn: {}, fnval: {}", copyedVideoInfo.getGuid(), qn, fnval);
    const auto result = biliClient.getPlayUrl(std::stoll(copyedVideoInfo.videoView->VideoId), qn, copyedVideoInfo.videoView->Identifier, fnval);
    if (result.code != 0)
    {
        BILIBILI_LOG_WARN("getPlayUrl error {}, error message: {}", result.code, result.message);
        return {};
    }

    std::list<std::string> video_urls;
    std::list<std::string> audio_urls;
    int needDownloadVideoId = 16;
    const auto& videos = result.data.dash.video;
    for (const auto& video : videos)
    {
        if (video.id <= qn && video.id > needDownloadVideoId)
        {
            needDownloadVideoId = video.id;
        }
    }

    for (const auto& video : videos)
    {
        if (video.id == needDownloadVideoId)
        {
            video_urls.push_back(video.baseUrl);
        }
    }

    int needDownloadAudioId = 30216;
    const auto& audios = result.data.dash.audio;
    for (const auto& audio : audios)
    {
        if (audio.id > needDownloadAudioId)
        {
            needDownloadAudioId = audio.id;
        }
    }

    for (const auto& audio : audios)
    {
        if (audio.id == needDownloadAudioId)
        {
            audio_urls.push_back(util::urlDecode(audio.baseUrl));
        }
    }

    download::ResourseInfo info;
    info.videoUris = video_urls;
    info.audioUris = audio_urls;
    auto fileName = videoInfo.fileName();
    info.option.out = fileName + ".mp4";
    info.option.dir = videoInfo.downloadConfig->downloadDir;
    const std::list<std::string> h = {"Referer: https://www.bilibili.com", std::string("User-Agent: ") + network::chrome};
    info.option.header = h;

    auto biliDownlaoder = std::shared_ptr<download::BiliDownloader>(new download::BiliDownloader(info), download::freeDownload);
    return biliDownlaoder;
}

LoginProxy BiliBiliPlugin::loginer()
{
    return LoginProxy(m_login);
}

void BiliBiliPlugin::setDir(std::string dir)
{
    m_dir = dir;
}

const std::string& BiliBiliPlugin::getDir()
{
    return m_dir;
}
