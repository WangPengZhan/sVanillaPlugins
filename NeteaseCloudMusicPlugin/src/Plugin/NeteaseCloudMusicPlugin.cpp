#include "NeteaseCloudMusicPlugin.h"

#include <Util/UrlProccess.h>

#include "NeteaseCloudMusicApi/NeteaseCloudMusicClient.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicLog.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicUrl.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicApi.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicConstants.h"
#include "NeteaseCloudMusicPluginMessage.h"
#include "NeteaseCloudMusicResource.h"
#include "NeteaseCloudMusicDownloader.h"
#include "Convert.h"

PluginMessage NeteaseCloudMusicPlugin::m_pluginMessage = {
    neteaseplugin::pluginID, neteaseplugin::name, neteaseplugin::version, neteaseplugin::description, neteaseplugin::home,
};

std::string NeteaseCloudMusicPlugin::m_dir;

NeteaseCloudMusicPlugin::NeteaseCloudMusicPlugin()
    : IPlugin()
    , m_client(netease::NeteaseCloudMusicClient::globalClient())
{
}

const PluginMessage& NeteaseCloudMusicPlugin::pluginMessage() const
{
    return m_pluginMessage;
}

const std::vector<uint8_t>& NeteaseCloudMusicPlugin::websiteIcon()
{
    return qrc_website_icon;
}

bool NeteaseCloudMusicPlugin::canParseUrl(const std::string& url)
{
    return isValidUrl(url);
}

adapter::VideoView NeteaseCloudMusicPlugin::getVideoView(const std::string& url)
{
    NETEASE_LOG_INFO("getVideoView url: {}", url);
    std::string id = getID(url);
    NETEASE_LOG_INFO("id: {}", id);
    if (id.empty())
    {
        NETEASE_LOG_ERROR("Invalid URL: {}", url);
        return {};
    }

    int pos = id.find(':');
    if (pos == std::string::npos)
    {
        NETEASE_LOG_ERROR("Invalid ID format: {}", id);
        return {};
    }

    netease::IDType type = netease::stringToIDType(id.substr(0, pos));
    uint64_t idNum = std::stoll(id.substr(pos + 1));

    adapter::VideoView views;

    switch (type)
    {
    case netease::IDType::Song:
    {
        auto songDetails = m_client.getSongDetails({idNum});
        views = convertVideoView(songDetails);
        break;
    }
    case netease::IDType::Album:
    {
        auto albumDetails = m_client.getAlbum(std::to_string(idNum));
        std::vector<uint64_t> ids;
        for (const auto& song : albumDetails.songs)
        {
            ids.push_back(song.id);
        }
        auto songDetails = m_client.getSongDetails(ids);
        views = convertVideoView(songDetails);
        break;
    }
    case netease::IDType::Playlist:
    {
        auto playlistDetails = m_client.getPlaylist(std::to_string(idNum));
        std::vector<uint64_t> ids;
        for (const auto& song : playlistDetails.playlist.trackIds)
        {
            ids.push_back(song.id);
        }
        auto songDetails = m_client.getSongDetails(ids);
        views = convertVideoView(songDetails);
        break;
    }
    case netease::IDType::MV:
    {
        auto mvDetails = m_client.getMVDetail(std::to_string(idNum));
        views = convertVideoView(mvDetails);
        break;
    }
    default:
        break;
    }

    return views;
}

std::shared_ptr<download::FileDownloader> NeteaseCloudMusicPlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    auto copyedVideoInfo = videoInfo;
    copyedVideoInfo.downloadConfig = std::make_shared<DownloadConfig>(*videoInfo.downloadConfig);
    copyedVideoInfo.videoView = std::make_shared<adapter::BaseVideoView>(*videoInfo.videoView);

    std::string url;
    auto type = static_cast<ContentType>(std::stoi(copyedVideoInfo.videoView->Identifier));
    switch (type)
    {
    case ContentType::MV:
    {
        auto data = m_client.getMVPlayUrl(copyedVideoInfo.videoView->Identifier);
        url = data.data.url;
        break;
    }
    case ContentType::SONG:
    {
        auto data = m_client.getSongPlayUrl({std::stoull(copyedVideoInfo.videoView->Identifier)}, netease::SoundLevel::Lossless);
        if (!data.data.empty())
        {
            url = data.data[0].url;
        }
        break;
    }
    default:
        break;
    }

    download::ResourceInfo info;
    info.videoUris = {url};
    auto fileName = videoInfo.fileName(true);
    info.option.out = fileName;
    info.option.dir = videoInfo.downloadConfig->downloadDir;
    const std::list<std::string> h = {std::string("Referer: ") + netease::home, std::string("User-Agent: ") + network::chrome};
    info.option.header = h;
    auto neteaseDownlaoder = std::shared_ptr<download::NeteaseCloudMusicDownloader>(new download::NeteaseCloudMusicDownloader(info), download::freeDownload);
    return neteaseDownlaoder;
}

LoginProxy NeteaseCloudMusicPlugin::loginer()
{
    return LoginProxy(m_login);
}

void NeteaseCloudMusicPlugin::setDir(std::string dir)
{
    m_dir = dir;
}

const std::string& NeteaseCloudMusicPlugin::getDir()
{
    return m_dir;
}
