#include "HLSDownloader.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string_view>
#include <utility>

#include "FFmpeg/FFmpegHelper.h"
#include "HLSApi/HLSClient.h"
#include "HLSApi/HLSLog.h"
#include "Plugin/HLSPlugin.h"
#include "Util/LocaleHelper.h"

namespace download
{
namespace
{
std::string createTempDirectory()
{
    static std::atomic_uint64_t counter{0};
    const auto root = std::filesystem::path(HLSPlugin::getDir());
    std::filesystem::create_directories(root);
    for (;;)
    {
        const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto candidate = root / ("HLS_" + std::to_string(tick) + "_" + std::to_string(counter.fetch_add(1)));
        std::error_code error;
        if (std::filesystem::create_directory(candidate, error))
        {
            return candidate.string() + std::string(1, std::filesystem::path::preferred_separator);
        }
        if (error && error != std::errc::file_exists)
        {
            throw std::filesystem::filesystem_error("Unable to create HLS temporary directory", candidate, error);
        }
    }
}

std::string_view renditionFilename(hlsapi::MediaInfo::MediaType type)
{
    switch (type)
    {
    case hlsapi::MediaInfo::Video:
        return "video.mp4";
    case hlsapi::MediaInfo::Audio:
        return "audio.m4a";
    case hlsapi::MediaInfo::Subtitle:
        return "subtitle.vtt";
    default:
        return {};
    }
}
}  // namespace

HLSDownloader::HLSDownloader()
    : m_tempDir(createTempDirectory())
{
}

HLSDownloader::HLSDownloader(std::shared_ptr<hlsapi::HLSClient> client, std::unordered_map<hlsapi::MediaInfo::MediaType, hlsapi::M3U8Playlist> playlists,
                             int downloadThreadNum)
    : m_tempDir(createTempDirectory())
    , m_client(std::move(client))
    , m_downloadThreadNum(downloadThreadNum)
{
    if (!m_client || playlists.empty())
    {
        HLS_LOG_ERROR("Cannot prepare HLS downloader without a client and playlists");
        m_setupValid = false;
        return;
    }
    for (auto& [type, playlist] : playlists)
    {
        auto downloader = std::make_shared<hlsapi::HLSPlaylistDownloader>(*m_client);
        downloader->setThreadCount(downloadThreadNum);
        downloader->setDir((std::filesystem::path(m_tempDir) / hlsapi::MediaInfo::mediaTypeToString(type)).string());
        if (!downloader->downloadTsToFiles({playlist}))
        {
            HLS_LOG_ERROR("Unable to queue HLS rendition, media type: {}", static_cast<int>(type));
            m_setupValid = false;
        }
        m_downloadedFiles[type] = std::move(downloader);
    }
}

HLSDownloader::~HLSDownloader()
{
    finish();
}

void HLSDownloader::start()
{
    if (!m_setupValid)
    {
        HLS_LOG_ERROR("Cannot start HLS download because setup failed");
        m_status = Error;
        return;
    }

    if (m_downloadedFiles.empty())
    {
        HLS_LOG_ERROR("Cannot start HLS download without queued renditions");
        return;
    }

    m_info.stage = "audio/video downloading";
    for (auto& [_, downloader] : m_downloadedFiles)
    {
        downloader->startDownload();
    }
    m_status = Downloading;
    HLS_LOG_INFO("HLS download started with {} rendition(s)", m_downloadedFiles.size());
}

void HLSDownloader::stop()
{
    for (auto& [_, downloader] : m_downloadedFiles)
    {
        downloader->stopDownload();
    }
    m_status = Waiting;
    HLS_LOG_INFO("HLS download stopped");
}

void HLSDownloader::pause()
{
    for (auto& [_, downloader] : m_downloadedFiles)
    {
        downloader->stopDownload();
    }
    m_status = Paused;
    HLS_LOG_INFO("HLS download paused");
}

void HLSDownloader::resume()
{
    if (!m_setupValid || m_finished)
    {
        HLS_LOG_ERROR("Cannot resume HLS download, setup valid: {}, finished: {}", m_setupValid, m_finished);
        return;
    }

    for (auto& [_, downloader] : m_downloadedFiles)
    {
        downloader->startDownload();
    }
    m_status = Downloading;
    HLS_LOG_INFO("HLS download resumed");
}

void HLSDownloader::downloadStatus()
{
    if (m_finished)
    {
        return;
    }
    m_info.total = 0;
    m_info.complete = 0;
    m_info.speed = 0;
    for (auto& [type, downloader] : m_downloadedFiles)
    {
        m_info.total += downloader->totalCount();
        m_info.complete += downloader->finishedCount();
        if (downloader->failedCount() > 0)
        {
            if (m_status != Error)
            {
                HLS_LOG_ERROR("HLS rendition download failed, media type: {}, failed segments: {}", static_cast<int>(type), downloader->failedCount());
            }
            m_status = Error;
            return;
        }
        if (m_mergedType.contains(type) || downloader->finishedCount() != downloader->totalCount() || downloader->totalCount() == 0)
        {
            continue;
        }

        const auto outputName = renditionFilename(type);
        if (outputName.empty())
        {
            HLS_LOG_ERROR("Unsupported HLS rendition media type: {}", static_cast<int>(type));
            m_status = Error;
            return;
        }
        const auto output = (std::filesystem::path(m_tempDir) / outputName).string();
        HLS_LOG_INFO("Merging HLS rendition, media type: {}", static_cast<int>(type));
        if (!downloader->mergeTs(output))
        {
            HLS_LOG_ERROR("Unable to merge HLS rendition, media type: {}", static_cast<int>(type));
            m_status = Error;
            return;
        }
        m_mergedType.insert(type);
        HLS_LOG_INFO("HLS rendition merged, media type: {}", static_cast<int>(type));
    }

    if (m_mergedType.size() != m_downloadedFiles.size())
    {
        return;
    }

    ffmpeg::MergeInfo merge;
    merge.targetVideo = util::localeToUtf8((std::filesystem::path(path()) / filename()).string());
    m_info.stage = "ffmpeg mixed";
    const auto mergedPath = [&](hlsapi::MediaInfo::MediaType type) {
        return util::localeToUtf8((std::filesystem::path(m_tempDir) / renditionFilename(type)).string());
    };
    if (m_mergedType.contains(hlsapi::MediaInfo::Video))
    {
        merge.video = mergedPath(hlsapi::MediaInfo::Video);
    }
    if (m_mergedType.contains(hlsapi::MediaInfo::Audio))
    {
        merge.audio = mergedPath(hlsapi::MediaInfo::Audio);
    }
    if (m_mergedType.contains(hlsapi::MediaInfo::Subtitle))
    {
        merge.subtitle = mergedPath(hlsapi::MediaInfo::Subtitle);
    }
    HLS_LOG_INFO("Creating final HLS output: {}", merge.targetVideo);
    if (!ffmpeg::FFmpegHelper::mergeVideo(merge))
    {
        HLS_LOG_ERROR("Unable to create final HLS output: {}", merge.targetVideo);
        m_status = Error;
        return;
    }
    m_finished = true;
    m_status = Finished;
    HLS_LOG_INFO("HLS output completed: {}", merge.targetVideo);
}

void HLSDownloader::finish()
{
    for (auto& [_, downloader] : m_downloadedFiles)
    {
        if (downloader)
        {
            downloader->exitDownload();
        }
    }
    std::error_code error;
    std::filesystem::remove_all(m_tempDir, error);
    if (error)
    {
        HLS_LOG_ERROR("Unable to clean HLS temporary directory: {}", error.message());
    }
}

void HLSDownloader::setPath(std::string path)
{
    const std::filesystem::path fsPath = util::utf8ToLocale(path);
    if (!std::filesystem::exists(fsPath))
    {
        std::filesystem::create_directories(fsPath);
    }
    m_path = std::move(path);
}

const std::string& HLSDownloader::path() const
{
    return m_path;
}

void HLSDownloader::setFilename(std::string filename)
{
    m_filename = std::move(filename);
}

const std::string& HLSDownloader::filename() const
{
    return m_filename;
}

bool HLSDownloader::isFinished() const
{
    return m_finished;
}

std::string HLSDownloader::nowDownload() const
{
    return {};
}

void HLSDownloader::addDownloadFile(hlsapi::MediaInfo::MediaType type, hlsapi::M3U8Playlist playlist)
{
    if (!playlist.isSupportedVod())
    {
        HLS_LOG_ERROR("Cannot add unsupported HLS playlist, media type: {}", static_cast<int>(type));
        m_setupValid = false;
        return;
    }
    if (!m_downloadedFiles.contains(type))
    {
        hlsapi::HLSClient& client = m_client ? *m_client : hlsapi::HLSClient::globalClient();
        m_downloadedFiles[type] =
            std::make_shared<hlsapi::HLSPlaylistDownloader>(client, (std::filesystem::path(m_tempDir) / hlsapi::MediaInfo::mediaTypeToString(type)).string());
    }
    m_downloadedFiles[type]->setThreadCount(m_downloadThreadNum);
    if (!m_downloadedFiles[type]->downloadTsToFiles({playlist}))
    {
        HLS_LOG_ERROR("Unable to queue HLS playlist, media type: {}", static_cast<int>(type));
        m_setupValid = false;
    }
}

void freeDownload(HLSDownloader* downloader)
{
    if (downloader)
    {
        delete downloader;
    }
}

}  // namespace download
