#include "BiliBiliPlugin.h"
#include "BiliApi/BilibiliClient.h"
#include "BiliApi/BiliApi.h"
#include "BiliApi/BilibiliUrl.h"
#include "BiliBiliResource.h"
#include "BiliBiliDownloader.h"
#include "Convert.h"
#include "BiliBiliPluginMessage.h"
#include "BiliApi/BilibiliLog.h"

#include <Util/UrlProccess.h>

PluginMessage BiliBiliPlugin::m_pluginMessage = {
    biliplugin::pluginID, biliplugin::name, biliplugin::version, biliplugin::description, biliplugin::domain,
};

std::string BiliBiliPlugin::m_dir;

BiliBiliPlugin::BiliBiliPlugin()
    : IPlugin()
    , m_client(biliapi::BilibiliClient::globalClient())
{
}

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
    return biliapi::isValidUrl(url);
}

adapter::VideoView BiliBiliPlugin::getVideoView(const std::string& url)
{
    BILIBILI_LOG_INFO("getVideoView url: {}", url);
    auto id = biliapi::getID(url);
    BILIBILI_LOG_INFO("id: {}", id.to_string());
    if (id.id.empty())
    {
        return {};
    }

    adapter::VideoView views;
    switch (id.type)
    {
    case biliapi::IDType::Aid:
    case biliapi::IDType::Bid:
    {
        auto videoView = m_client.getVideoView(id.id, id.type);
        views = convertVideoView(videoView.data);
        break;
    }
    case biliapi::IDType::BangumiSS:
    case biliapi::IDType::BangumiEP:
    {
        auto videoView = m_client.getSeasonVideoView(id.id, id.type);
        views = convertVideoView(videoView.result);
        break;
    }
    case biliapi::IDType::BangumiMD:
    {
        auto mdInfo = m_client.getMdVideoView(id.id);
        std::string season_id = std::to_string(mdInfo.result.media.season_id);
        auto videoView = m_client.getSeasonVideoView(season_id, biliapi::IDType::BangumiSS);
        views = convertVideoView(videoView.result);
        break;
    }
    case biliapi::IDType::CheeseSS:
    case biliapi::IDType::CheeseEP:
    {
        auto videoView = m_client.getCheeseVideoView(id.id, id.type);
        views = convertVideoView(videoView.data);
        break;
    }
    case biliapi::IDType::FavoritesId:
    {
        auto info = m_client.getFavInfo(id.id);
        auto videoView = m_client.getFavDetail(id.id);
        auto videoInfos = m_client.getFavVideoInfo(videoView.data, info.data.mid, info.data.id);
        for (const auto& data : videoInfos.data)
        {
            views.push_back(convertVideoInfo(data));
        }
        break;
    }
    case biliapi::IDType::UserId:
    {
        auto info = m_client.getUserVideoWroks(id.id);
        views = convertVideoView(info.data);
        std::map<std::string, std::string> bvMap;
        for (auto& view : views)
        {
            if (bvMap.find(view.Identifier) == bvMap.end())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 255));
                auto videoView = m_client.getVideoView(view.Identifier, biliapi::IDType::Bid);
                auto detailViews = convertVideoView(videoView.data);
                for (const auto& detailView : detailViews)
                {
                    if (detailView.Identifier == view.Identifier)
                    {
                        view.Option2 = detailView.Option2;
                    }
                    bvMap[detailView.Identifier] = detailView.Option2;
                }   
            }
            else
            {
                view.Option2 = bvMap[view.Identifier];
            }
        }

        break;
    }
    default:
        break;
    }

    return views;
}

std::shared_ptr<download::FileDownloader> BiliBiliPlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    auto copyedVideoInfo = videoInfo;
    copyedVideoInfo.downloadConfig = std::make_shared<DownloadConfig>(*videoInfo.downloadConfig);
    copyedVideoInfo.videoView = std::make_shared<adapter::BaseVideoView>(*videoInfo.videoView);

    auto& biliClient = m_client;
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

    biliapi::PlayDash dash;
    auto idType = biliapi::stringToType(copyedVideoInfo.videoView->IdType);
    if (idType == biliapi::IDType::BangumiEP)
    {
        const auto result = biliClient.getPlayUrl(std::stoll(copyedVideoInfo.videoView->Identifier), idType, qn, fnval);
        if (result.code != 0)
        {
            BILIBILI_LOG_WARN("getPlayUrl error {}, error message: {}", result.code, result.message);
            return {};
        }
        dash = result.result.dash;
    }
    else if (idType == biliapi::IDType::CheeseEP)
    {
        const auto result = biliClient.getPlayUrl(std::stoll(copyedVideoInfo.videoView->Option1), std::stoll(copyedVideoInfo.videoView->Identifier),
                                                  std::stoll(copyedVideoInfo.videoView->Identifier), qn, fnval);
        if (result.code != 0)
        {
            BILIBILI_LOG_WARN("getPlayUrl error {}, error message: {}", result.code, result.message);
            return {};
        }
        dash = result.data.dash;
    }
    else
    {
        const auto result = biliClient.getPlayUrl(std::stoll(copyedVideoInfo.videoView->Option2), qn, copyedVideoInfo.videoView->Identifier, fnval);
        if (result.code != 0)
        {
            BILIBILI_LOG_WARN("getPlayUrl error {}, error message: {}", result.code, result.message);
            return {};
        }
        dash = result.data.dash;
    }

    std::list<std::string> video_urls;
    std::list<std::string> audio_urls;
    int needDownloadVideoId = 16;
    const auto& videos = dash.video;
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
            video_urls.push_back(video.baseUrl.empty() ? util::urlDecode(video.base_url) : util::urlDecode(video.baseUrl));
        }
    }

    int needDownloadAudioId = 30216;
    const auto& audios = dash.audio;
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
            audio_urls.push_back(audio.baseUrl.empty() ? util::urlDecode(audio.base_url) : util::urlDecode(audio.baseUrl));
        }
    }

    download::ResourceInfo info;
    info.videoUris = video_urls;
    info.audioUris = audio_urls;
    auto fileName = videoInfo.fileName(true);
    info.option.out = fileName;
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
