#include "WeiboPlugin.h"
#include "WeiboApi/WeiboClient.h"
#include "WeiboApi/WeiboApi.h"
#include "WeiboApi/WeiboUrl.h"
#include "WeiboApi/WeiboApiConstants.h"
#include "WeiboResource.h"
#include "WeiboDownloader.h"
#include "Convert.h"
#include "WeiboPluginMessage.h"
#include "WeiboApi/WeiboLog.h"
#include "Util/LocaleHelper.h"

#include <Util/UrlProccess.h>

PluginMessage WeiboPlugin::m_pluginMessage = {
    weiboplugin::pluginID, weiboplugin::name, weiboplugin::version, weiboplugin::description, weiboplugin::domain,
};

std::string WeiboPlugin::m_dir;

WeiboPlugin::WeiboPlugin()
    : IPlugin()
    , m_client(weiboapi::WeiboClient::globalClient())
{
}

const PluginMessage& WeiboPlugin::pluginMessage() const
{
    return m_pluginMessage;
}

const std::vector<uint8_t>& WeiboPlugin::websiteIcon()
{
    return qrc_website_icon;
}

bool WeiboPlugin::canParseUrl(const std::string& url)
{
    return isValidUrl(url);
}

adapter::VideoView WeiboPlugin::getVideoView(const std::string& url)
{
    WEIBO_LOG_INFO("getVideoView url: {}", url);
    std::string id = getID(url);
    WEIBO_LOG_INFO("id: {}", id);
    adapter::VideoView views;

    if (id.starts_with("mid"))
    {
        id = id.substr(4);
        auto ret = m_client.getPlayInfoByMid(id);
        views = convertVideoView(ret);
    }
    else
    {
        id = id.substr(4);
        auto ret = m_client.getPlayInfoByWid(id);
        views = convertVideoView(ret);
    }

    for (auto& view : views)
    {
        std::string path = m_dir + "cover/" + view.VideoId + std::to_string(std::rand() % 10000) + ".jpg";
        if (!std::filesystem::exists(std::filesystem::path(path).parent_path()))
        {
            std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        }

        if (m_client.downloadImage(view.Cover, path))
        {
            view.Cover = util::localeToUtf8(path);
        }
    }

    return views;
}

std::shared_ptr<download::FileDownloader> WeiboPlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    auto copyedVideoInfo = videoInfo;
    copyedVideoInfo.downloadConfig = std::make_shared<DownloadConfig>(*videoInfo.downloadConfig);
    copyedVideoInfo.videoView = std::make_shared<adapter::BaseVideoView>(*videoInfo.videoView);

    const auto result = m_client.getStreamInfo(copyedVideoInfo.videoView->Identifier);
    if (result.empty())
    {
        return {};
    }

    download::ResourseInfo info;
    std::string videoUrl = result;

    info.videoUris = {videoUrl};
    auto fileName = videoInfo.fileName(true);
    info.option.out = fileName;
    info.option.dir = videoInfo.downloadConfig->downloadDir;

    std::list<std::string> h = {std::string("User-Agent: ") + network::chrome, std::string("Referer: ") + weiboapi::weiboHomeUrl};
    if (m_client.isLogined() && !m_client.cookie().empty())
    {
        h.push_back("Cookie: " + m_client.cookie());
    }
    info.option.header = h;

    auto weiboDownlaoder = std::shared_ptr<download::WeiboDownloader>(new download::WeiboDownloader(info), download::freeDownload);
    return weiboDownlaoder;
}

LoginProxy WeiboPlugin::loginer()
{
    return LoginProxy(m_login);
}

void WeiboPlugin::setDir(std::string dir)
{
    m_dir = dir;
}

const std::string& WeiboPlugin::getDir()
{
    return m_dir;
}
