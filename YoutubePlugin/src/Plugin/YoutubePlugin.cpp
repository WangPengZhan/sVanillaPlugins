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
    return youtubeapi::isValidUrl(url);
}

adapter::VideoView YoutubePlugin::getVideoView(const std::string& url)
{
    adapter::VideoView views;
    youtubeapi::IDInfo id = youtubeapi::getID(url);
    auto& client = youtubeapi::YoutubeClient::globalClient();
    if (id.type == youtubeapi::IDType::VideoId && id.parentType != youtubeapi::IDType::PlaylistId)
    {
        youtubeapi::MainResponse mainResponse = client.getVideoInfo(id.id);
        client.getBaseJs(client.getIFrameVersion());
        views = convertVideoView(mainResponse);
    }
    else if (id.type == youtubeapi::IDType::PlaylistId || id.parentType == youtubeapi::IDType::PlaylistId)
    {
        std::string playlistId = id.type == youtubeapi::IDType::PlaylistId ? id.id : id.parentId;
        youtubeapi::PlayListInfo playlistInfo = client.playlistInfo(playlistId);
        views = convertVideoView(playlistInfo.contents);
    }
    else if (id.type == youtubeapi::IDType::ChannelId)
    {
        std::string playlistUrl = "https://www.youtube.com/playlist?list=UU" + id.id.substr(2);
        views = getVideoView(playlistUrl);
    }
    else if (id.type == youtubeapi::IDType::AtChannelId)
    {
        std::string requestUrl = "https://www.youtube.com/@" + id.id;
        std::string ChannelUrl = client.getChannelUrl(requestUrl);
        views = getVideoView(ChannelUrl);
    }
    else if (id.type == youtubeapi::IDType::UserNameId)
    {
        std::string requestUrl = "https://www.youtube.com/user/" + id.id;
        std::string ChannelUrl = client.getChannelUrl(requestUrl);
        views = getVideoView(ChannelUrl);
    }
    else if (id.type == youtubeapi::IDType::CustomNameId)
    {
        std::string requestUrl = "https://www.youtube.com/c/" + id.id;
        std::string ChannelUrl = client.getChannelUrl(requestUrl);
        views = getVideoView(ChannelUrl);
    }
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
