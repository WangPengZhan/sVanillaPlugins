#include "YoutubePlugin.h"
#include "YoutubeApi/YoutubeClient.h"
#include "YoutubeApi/YoutubeApi.h"
#include "YoutubeApi/YoutubeUrl.h"
#include "YoutubeApi/YoutubeConstants.h"
#include "YoutubeResource.h"
#include "YoutubeDownloader.h"
#include "Convert.h"
#include "YoutubePluginMessage.h"

#include <Util/UrlProccess.h>

PluginMessage YoutubePlugin::m_pluginMessage = {
    youtubeplugin::pluginID, youtubeplugin::name, youtubeplugin::version, youtubeplugin::description, youtubeplugin::domain,
};
std::string YoutubePlugin::m_dir;

const PluginMessage& YoutubePlugin::pluginMessage() const
{
    return m_pluginMessage;
}

const std::vector<uint8_t>& YoutubePlugin::websiteIcon()
{
    return qrc_website_icon;
}

bool YoutubePlugin::canParseUrl(const std::string& url)
{
    return isValidUrl(url);
}

adapter::VideoView YoutubePlugin::getVideoView(const std::string& url)
{
    adapter::VideoView views;

    std::string id = getID(url);
    auto& client = youtubeapi::YoutubeClient::globalClient();
    auto mainResponse = client.getVideoInfo(id);
    client.getBaseJs(client.getIFrameVersion());
    views = convertVideoView(mainResponse);

    return views;
}

std::shared_ptr<download::FileDownloader> YoutubePlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    auto copyedVideoInfo = videoInfo;
    copyedVideoInfo.downloadConfig = std::make_shared<DownloadConfig>(*videoInfo.downloadConfig);
    copyedVideoInfo.videoView = std::make_shared<adapter::BaseVideoView>(*videoInfo.videoView);

    auto& youtubeClient = youtubeapi::YoutubeClient::globalClient();
    const auto result = youtubeClient.getStreamInfo(copyedVideoInfo.videoView->Identifier);
    if (result.empty())
    {
        return {};
    }

    download::ResourceInfo info;
    std::string videoUrl = result.front().url + "&range=0-" + result.front().contentLength;
    if (result.size() != 1)
    {
        std::string audioUrl = result.back().url + "&range=0-" + result.back().contentLength;
        info.audioUris = {audioUrl};
    }

    info.videoUris = {videoUrl};
    auto fileName = videoInfo.fileName(true);
    info.option.out = fileName;
    info.option.dir = videoInfo.downloadConfig->downloadDir;
    const std::list<std::string> h = {youtubeapi::youtube_referer, youtubeapi::youtube_user_agent};
    info.option.header = h;

    auto youtubeDownloader = std::shared_ptr<download::YoutubeDownloader>(new download::YoutubeDownloader(info), download::freeDownload);
    return youtubeDownloader;
}

LoginProxy YoutubePlugin::loginer()
{
    return LoginProxy(m_login);
}

void YoutubePlugin::setDir(std::string dir)
{
    m_dir = dir;
}

const std::string& YoutubePlugin::getDir()
{
    return m_dir;
}
