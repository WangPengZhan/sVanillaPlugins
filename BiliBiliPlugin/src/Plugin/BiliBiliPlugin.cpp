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
#include <Util/NumberParse.h>

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
        if (mdInfo.code != 0 || mdInfo.result.media.season_id == 0)
        {
            BILIBILI_LOG_WARN("getMdVideoView failed or empty season, media_id: {}, code: {}", id.id, mdInfo.code);
            break;
        }
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
                    if (detailView.Identifier == view.Identifier && view.Option2.empty())
                    {
                        view.Option2 = detailView.Option2;
                    }

                    if (bvMap.find(detailView.Identifier) == bvMap.end())
                    {
                        bvMap[detailView.Identifier] = detailView.Option2;
                    }
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

    if (id.type == biliapi::IDType::Aid || id.type == biliapi::IDType::Bid || id.type == biliapi::IDType::BangumiEP || id.type == biliapi::IDType::CheeseEP)
    {
        prioritizeVideoView(views, id);
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

    long long fnval = 16;
    BILIBILI_LOG_INFO("getDownloader, guid: {}, qn: {}, fnval: {}", copyedVideoInfo.getGuid(), qn, fnval);

    const auto& view = *copyedVideoInfo.videoView;
    biliapi::PlayDash dash;
    auto idType = biliapi::stringToType(view.IdType);
    if (idType == biliapi::IDType::BangumiEP)
    {
        long long epId = 0;
        if (!util::parseNumber(view.Identifier, epId))
        {
            BILIBILI_LOG_WARN("getDownloader: invalid bangumi ep id: {}", view.Identifier);
            return {};
        }

        const auto result = biliClient.getPlayUrl(epId, idType, qn, fnval);
        if (result.code != 0)
        {
            BILIBILI_LOG_WARN("getPlayUrl error {}, error message: {}", result.code, result.message);
            return {};
        }
        dash = result.result.dash;
    }
    else if (idType == biliapi::IDType::CheeseEP)
    {
        long long avid = 0;
        long long epId = 0;
        long long cid = 0;
        if (!util::parseNumber(view.Option1, avid) || !util::parseNumber(view.Identifier, epId) || !util::parseNumber(view.Option2, cid))
        {
            BILIBILI_LOG_WARN("getDownloader: invalid cheese ids, aid: {}, ep: {}, cid: {}", view.Option1, view.Identifier, view.Option2);
            return {};
        }

        const auto result = biliClient.getPlayUrl(avid, epId, cid, qn, fnval);
        if (result.code != 0)
        {
            BILIBILI_LOG_WARN("getPlayUrl error {}, error message: {}", result.code, result.message);
            return {};
        }
        dash = result.data.dash;
    }
    else
    {
        long long cid = 0;
        if (!util::parseNumber(view.Option2, cid))
        {
            BILIBILI_LOG_WARN("getDownloader: invalid cid: {} for bvid: {}", view.Option2, view.Identifier);
            return {};
        }

        const auto result = biliClient.getPlayUrl(cid, qn, view.Identifier, fnval);
        if (result.code != 0)
        {
            BILIBILI_LOG_WARN("getPlayUrl error {}, error message: {}", result.code, result.message);
            return {};
        }
        dash = result.data.dash;
    }

    BILIBILI_LOG_INFO("getPlayUrl streams: dashVideo={}, dashAudio={}", dash.video.size(), dash.audio.size());
    if (dash.video.empty())
    {
        BILIBILI_LOG_WARN("getPlayUrl returned no DASH video streams");
        return {};
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

    // Prefer the standard DASH audio qualities (192K > 132K > 64K) instead of
    // blindly taking the largest id, which may be a Dolby/Hi-Res stream that
    // requires a paid membership.
    static const int audioQualityPriority[] = {30280, 30232, 30216};
    const auto& audios = dash.audio;
    int needDownloadAudioId = 0;
    for (const int quality : audioQualityPriority)
    {
        for (const auto& audio : audios)
        {
            if (audio.id == quality)
            {
                needDownloadAudioId = audio.id;
                break;
            }
        }

        if (needDownloadAudioId != 0)
        {
            break;
        }
    }

    if (needDownloadAudioId == 0 && !audios.empty())
    {
        needDownloadAudioId = audios.front().id;
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
