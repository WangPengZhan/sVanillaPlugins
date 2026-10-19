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

namespace
{
std::string appendRange(const youtubeapi::AdaptiveFormat& format)
{
    if (format.contentLength.empty())
    {
        return format.url;
    }

    const char separator = format.url.find('?') == std::string::npos ? '?' : '&';
    return format.url + separator + "range=0-" + format.contentLength;
}
}  // namespace

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
        views = convertVideoView(mainResponse);
    }
    else if (id.type == youtubeapi::IDType::PlaylistId || id.parentType == youtubeapi::IDType::PlaylistId)
    {
        std::string playlistId = id.type == youtubeapi::IDType::PlaylistId ? id.id : id.parentId;
        youtubeapi::PlayListInfo playlistInfo = client.playlistInfo(playlistId);
        views = convertVideoView(playlistInfo.contents);
        if (id.type == youtubeapi::IDType::VideoId)
        {
            prioritizeVideoView(views, id.id);
        }
    }
    else if (id.type == youtubeapi::IDType::ChannelId)
    {
        if (id.id.size() <= 2 || !id.id.starts_with("UC"))
        {
            return views;
        }

        std::string playlistUrl = "https://www.youtube.com/playlist?list=UU" + id.id.substr(2);
        views = getVideoView(playlistUrl);
    }
    else if (id.type == youtubeapi::IDType::AtChannelId || id.type == youtubeapi::IDType::UserNameId || id.type == youtubeapi::IDType::CustomNameId)
    {
        std::string requestUrl = "https://www.youtube.com";
        if (id.type == youtubeapi::IDType::AtChannelId)
        {
            requestUrl += "/@";
        }
        else if (id.type == youtubeapi::IDType::UserNameId)
        {
            requestUrl += "/user/";
        }
        else
        {
            requestUrl += "/c/";
        }
        requestUrl += id.id;

        const std::string channelUrl = client.getChannelUrl(requestUrl);
        const auto resolvedId = youtubeapi::getID(channelUrl);
        // A failed lookup may return an alias URL again; do not recurse on aliases.
        if (resolvedId.type == youtubeapi::IDType::ChannelId || resolvedId.type == youtubeapi::IDType::PlaylistId)
        {
            views = getVideoView(channelUrl);
        }
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
    std::string videoUrl = appendRange(result.front());
    if (result.size() != 1)
    {
        std::string audioUrl = appendRange(result.back());
        info.audioUris = {audioUrl};
    }

    info.videoUris = {videoUrl};
    auto fileName = videoInfo.fileName(true);
    info.option.out = fileName;
    info.option.dir = videoInfo.downloadConfig->downloadDir;
    const std::list<std::string> h = {youtubeapi::youtube_referer, youtubeapi::youtube_player_user_agent};
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
