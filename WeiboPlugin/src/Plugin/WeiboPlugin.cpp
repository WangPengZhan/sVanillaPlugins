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

#include <functional>
#include <Util/UrlProccess.h>

namespace
{
std::string coverFileName(const adapter::BaseVideoView& view)
{
    const std::string invalidChars = "\\/:*?\"<>|";
    std::string name = view.Identifier;
    for (char& character : name)
    {
        if (invalidChars.find(character) != std::string::npos)
        {
            character = '_';
        }
    }

    return name + "_" + std::to_string(std::hash<std::string>{}(view.Cover)) + ".jpg";
}
}  // namespace

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
    return weiboapi::isValidUrl(url);
}

adapter::VideoView WeiboPlugin::getVideoView(const std::string& url)
{
    WEIBO_LOG_INFO("getVideoView url: {}", url);
    const weiboapi::IDInfo id = weiboapi::getID(url);
    WEIBO_LOG_INFO("id: {}", id.to_string());
    if (id.id.empty())
    {
        WEIBO_LOG_ERROR("cannot resolve a video id from url: {}", url);
        return {};
    }

    adapter::VideoView views;
    switch (id.type)
    {
    case weiboapi::IDType::Status:
        views = convertVideoView(m_client.getPlayInfoByWid(id.id));
        break;
    case weiboapi::IDType::TV:
        views = convertVideoView(m_client.getPlayInfoByMid(id.id));
        break;
    default:
        WEIBO_LOG_ERROR("unsupported id type for url: {}", url);
        return {};
    }

    if (views.empty())
    {
        WEIBO_LOG_ERROR("no video view resolved for id: {}", id.id);
        return views;
    }

    const std::string idType = weiboapi::typeToString(id.type);
    for (auto& view : views)
    {
        view.IdType = idType;
        if (view.Cover.empty() || view.Identifier.empty())
        {
            continue;
        }

        std::string path = m_dir + "cover/" + coverFileName(view);
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

    download::ResourceInfo info;
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
