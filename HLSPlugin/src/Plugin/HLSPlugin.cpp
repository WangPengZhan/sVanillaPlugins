#include "HLSPlugin.h"

#include <chrono>
#include <filesystem>
#include <unordered_map>

#include "FFmpeg/FFmpegHelper.h"
#include "HLSApi/HLSClient.h"
#include "HLSApi/HLSConstants.h"
#include "HLSApi/HLSLog.h"
#include "HLSApi/HLSParser.h"
#include "HLSDownloader.h"
#include "HLSPluginMessage.h"
#include "HLSResource.h"
#include "Util/LocaleHelper.h"
#include "Util/NumberParse.h"
#include "Util/TimerUtil.h"

namespace
{
std::string_view getUrlLast(std::string_view url)
{
    if (const auto query = url.find_first_of("?#"); query != std::string_view::npos)
    {
        url.remove_suffix(url.size() - query);
    }
    const auto slash = url.rfind('/');
    return slash == std::string_view::npos ? url : url.substr(slash + 1);
}

bool parseManifest(hlsapi::HLSClient& client, const std::string& url, hlsapi::HLSParser& parser)
{
    std::string effectiveUrl;
    const std::string manifest = client.downloadM3u8(url, &effectiveUrl);
    const auto content = hlsapi::trim(manifest);
    if (content.empty())
    {
        HLS_LOG_ERROR("HLS manifest is empty");
        return false;
    }
    parser.setOriginUri(effectiveUrl.empty() ? url : effectiveUrl);
    if (!parser.parse(content))
    {
        HLS_LOG_ERROR("Unable to parse HLS manifest");
        return false;
    }
    return true;
}

const hlsapi::StreamInfo* selectVariant(const hlsapi::M3U8Master& master)
{
    const hlsapi::StreamInfo* selected = nullptr;
    for (const auto& stream : master.streams)
    {
        if (stream.isVideoStream() && (!selected || stream.bandwidth > selected->bandwidth))
        {
            selected = &stream;
        }
    }
    return selected;
}

bool loadPlaylist(hlsapi::HLSClient& client, const std::string& url, hlsapi::M3U8Playlist& playlist)
{
    hlsapi::HLSParser parser;
    if (!parseManifest(client, url, parser))
    {
        return false;
    }
    if (parser.isMaster())
    {
        HLS_LOG_ERROR("Expected an HLS media playlist, received a master playlist");
        return false;
    }
    playlist = parser.playlist();
    if (playlist.segments.empty())
    {
        HLS_LOG_ERROR("HLS media playlist contains no segments");
        return false;
    }
    return true;
}

bool downloadKeys(hlsapi::HLSClient& client, hlsapi::M3U8Playlist& playlist)
{
    const auto downloadKey = [&](hlsapi::KeyInfo& key) {
        if (key.method == hlsapi::KeyInfo::None)
        {
            return true;
        }
        key.key.clear();
        const auto uri = key.getUri();
        if (uri.empty())
        {
            HLS_LOG_ERROR("Encrypted HLS item has no key URI");
            return false;
        }
        return client.downloadKey(uri, key.key);
    };
    for (auto& segment : playlist.segments)
    {
        if (!downloadKey(segment.keyInfo))
        {
            return false;
        }
    }
    for (auto& map : playlist.mediaMaps)
    {
        if (!downloadKey(map.keyInfo))
        {
            return false;
        }
    }
    if (!playlist.segments.empty())
    {
        playlist.keyInfo = playlist.segments.back().keyInfo;
    }
    return true;
}

bool cutImage(hlsapi::M3U8Playlist playlist, const std::string& imagePath)
{
    if (playlist.segments.empty())
    {
        HLS_LOG_ERROR("Cannot create HLS cover from an empty playlist");
        return false;
    }
    auto& client = hlsapi::HLSClient::globalClient();
    if (!downloadKeys(client, playlist))
    {
        return false;
    }
    const auto& segment = playlist.segments[playlist.segments.size() / 2];
    const auto tempDir = std::filesystem::path(HLSPlugin::getDir()) / "temp";
    std::filesystem::create_directories(tempDir);
    const auto tsPath = tempDir / ("cover_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".ts");
    if (!client.downloadTsToFiles(segment.getUri(), util::localeToUtf8(tsPath.string()), segment.keyInfo, segment.mediaSequence))
    {
        HLS_LOG_ERROR("Unable to download HLS cover segment");
        return false;
    }
    std::vector<std::string> arguments = {"-i", util::localeToUtf8(tsPath.string()), "-ss", "00:00:00", "-vframes", "1", util::localeToUtf8(imagePath), "-y"};
    const bool result = ffmpeg::FFmpegHelper::globalInstance().startFFmpeg(arguments, [] {}, [] {});
    std::error_code error;
    std::filesystem::remove(tsPath, error);
    if (!result || !std::filesystem::exists(imagePath))
    {
        HLS_LOG_ERROR("Unable to extract HLS cover image");
        return false;
    }
    return true;
}

bool validFilename(std::string_view filename)
{
    return !filename.empty() && filename.find_first_of("/\\:\r\n") == std::string_view::npos;
}

const hlsapi::MediaInfo* selectRendition(const hlsapi::M3U8Master& master, hlsapi::MediaInfo::MediaType type, std::string_view group)
{
    const hlsapi::MediaInfo* selected = nullptr;
    for (const auto& media : master.mediaInfos)
    {
        if (media.type != type || media.groupId != group || media.uri.empty())
        {
            continue;
        }
        if (!selected || media.strDefault)
        {
            selected = &media;
        }
    }
    return selected;
}

using Playlists = std::unordered_map<hlsapi::MediaInfo::MediaType, hlsapi::M3U8Playlist>;

bool loadOptionalRendition(hlsapi::HLSClient& client, const hlsapi::M3U8Master& master, hlsapi::MediaInfo::MediaType type, std::string_view group,
                           Playlists& playlists)
{
    if (group.empty())
    {
        return true;
    }
    const auto* rendition = selectRendition(master, type, group);
    if (!rendition)
    {
        return true;
    }
    hlsapi::M3U8Playlist playlist;
    if (!loadPlaylist(client, rendition->getUri(), playlist))
    {
        HLS_LOG_ERROR("Unable to load HLS rendition, media type: {}", static_cast<int>(type));
        return false;
    }
    playlists.emplace(type, std::move(playlist));
    return true;
}

bool loadDownloadPlaylists(hlsapi::HLSClient& client, const hlsapi::HLSParser& parser, Playlists& playlists)
{
    if (!parser.isMaster())
    {
        playlists.emplace(hlsapi::MediaInfo::Video, parser.playlist());
        return true;
    }

    const auto& master = parser.master();
    const auto* variant = selectVariant(master);
    if (!variant)
    {
        HLS_LOG_ERROR("HLS master playlist has no video variant");
        return false;
    }
    hlsapi::M3U8Playlist video;
    if (!loadPlaylist(client, variant->getUri(), video))
    {
        return false;
    }
    playlists.emplace(hlsapi::MediaInfo::Video, std::move(video));
    return loadOptionalRendition(client, master, hlsapi::MediaInfo::Audio, variant->audio, playlists) &&
           loadOptionalRendition(client, master, hlsapi::MediaInfo::Subtitle, variant->subtitles, playlists);
}
}  // namespace

std::string HLSPlugin::m_dir;

PluginMessage HLSPlugin::m_pluginMessage = {
    hlsplugin::pluginID, hlsplugin::name, hlsplugin::version, hlsplugin::description, hlsplugin::domain,
};

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
    try
    {
        hlsapi::HLSClient& client = hlsapi::HLSClient::globalClient();
        hlsapi::HLSParser parser;
        if (!parseManifest(client, url, parser))
        {
            return {};
        }

        hlsapi::M3U8Playlist playlist;
        if (parser.isMaster())
        {
            const auto* stream = selectVariant(parser.master());
            if (!stream || !loadPlaylist(client, stream->getUri(), playlist))
            {
                if (!stream)
                {
                    HLS_LOG_ERROR("HLS master playlist has no video variant for view");
                }
                return {};
            }
        }
        else
        {
            playlist = parser.playlist();
        }
        if (playlist.segments.empty())
        {
            HLS_LOG_ERROR("Cannot create HLS view from an empty playlist");
            return {};
        }

        adapter::BaseVideoView view;
        view.Identifier = url;
        const auto& firstSegment = playlist.segments.front();
        view.Title = firstSegment.title.empty() ? std::string(getUrlLast(url)) : firstSegment.title;
        view.Duration = formatDuration(playlist.durationAll());
        view.pluginId = pluginMessage().pluginId;

        const auto coverPath =
            std::filesystem::path(m_dir) / "cover" / ("HLS_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".jpg");
        std::filesystem::create_directories(coverPath.parent_path());
        if (cutImage(playlist, coverPath.string()))
        {
            view.Cover = util::localeToUtf8(coverPath.string());
        }
        return {view};
    }
    catch (const std::exception& error)
    {
        HLS_LOG_ERROR("Unable to create HLS view: {}", error.what());
        return {};
    }
}

std::shared_ptr<download::FileDownloader> HLSPlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    try
    {
        if (!videoInfo.downloadConfig || !videoInfo.videoView)
        {
            HLS_LOG_ERROR("Cannot create HLS downloader without download configuration and video view");
            return {};
        }
        int threadCount = 5;
        if (!videoInfo.videoView->Option1.empty() && (!util::parseNumber(videoInfo.videoView->Option1, threadCount) || threadCount < 1 || threadCount > 20))
        {
            HLS_LOG_ERROR("Invalid HLS download thread count: {}", videoInfo.videoView->Option1);
            return {};
        }

        auto client = std::make_shared<hlsapi::HLSClient>();
        hlsapi::HLSParser parser;
        if (!parseManifest(*client, videoInfo.videoView->Identifier, parser))
        {
            return {};
        }

        Playlists playlists;
        if (!loadDownloadPlaylists(*client, parser, playlists))
        {
            return {};
        }

        for (auto& [_, playlist] : playlists)
        {
            if (!playlist.isSupportedVod())
            {
                HLS_LOG_ERROR("HLS playlist is not a supported VOD");
                return {};
            }
            if (!downloadKeys(*client, playlist))
            {
                return {};
            }
        }

        auto downloader = std::shared_ptr<download::HLSDownloader>(new download::HLSDownloader(client, playlists, threadCount), download::freeDownload);
        downloader->setPath(videoInfo.downloadConfig->downloadDir);
        auto filename = videoInfo.fileName();
        if (filename.empty())
        {
            filename = std::string(getUrlLast(videoInfo.videoView->Identifier));
        }
        if (!validFilename(filename))
        {
            HLS_LOG_ERROR("Invalid HLS output filename");
            return {};
        }
        downloader->setFilename(std::move(filename));
        return downloader;
    }
    catch (const std::exception& error)
    {
        HLS_LOG_ERROR("Unable to create HLS downloader: {}", error.what());
        return {};
    }
}

LoginProxy HLSPlugin::loginer()
{
    return LoginProxy(m_login);
}

void HLSPlugin::setDir(std::string dir)
{
    m_dir = std::move(dir);
}

const std::string& HLSPlugin::getDir()
{
    return m_dir;
}
