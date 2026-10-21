#include "HLSPlaylistDownloader.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "FFmpeg/FFmpegHelper.h"
#include "HLSClient.h"
#include "HLSLog.h"
#include "Util/LocaleHelper.h"

namespace hlsapi
{
HLSPlaylistDownloader::HLSPlaylistDownloader(HLSClient& client, const std::string& dir)
    : HLSPlaylistDownloader(client)
{
    setDir(dir);
}

HLSPlaylistDownloader::HLSPlaylistDownloader(HLSClient& client)
    : m_client(client)
{
}

HLSPlaylistDownloader::~HLSPlaylistDownloader()
{
    exitDownload();
}

HLSClient& HLSPlaylistDownloader::client() const
{
    return m_client;
}

void HLSPlaylistDownloader::setDir(const std::string& dir)
{
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
    }
    m_dir = dir;
}

const std::string& HLSPlaylistDownloader::dir() const
{
    return m_dir;
}

bool HLSPlaylistDownloader::downloadTsToFiles(const std::vector<M3U8Playlist>& playlists)
{
    if (playlists.empty())
    {
        HLS_LOG_ERROR("Cannot queue an empty HLS playlist batch");
        return false;
    }
    if (playlists.size() != 1 && std::any_of(playlists.begin(), playlists.end(), [](const M3U8Playlist& playlist) {
            return playlist.hasMap;
        }))
    {
        HLS_LOG_ERROR("Cannot queue mapped HLS playlists in a multi-playlist batch");
        return false;
    }
    for (const auto& playlist : playlists)
    {
        if (!playlist.isSupportedVod() || playlist.segments.empty())
        {
            HLS_LOG_ERROR("Cannot queue an unsupported or empty HLS media playlist");
            return false;
        }
        for (const auto& segment : playlist.segments)
        {
            if (segment.getUri().empty())
            {
                HLS_LOG_ERROR("Cannot queue an HLS segment without a URI");
                return false;
            }
        }
    }

    std::lock_guard lock(m_mutex);
    if (m_exit || m_dir.empty())
    {
        HLS_LOG_ERROR("Cannot queue HLS segments after exit or without a download directory");
        return false;
    }
    if (!m_taskFiles.empty() && (m_hasMap || playlists.front().hasMap))
    {
        HLS_LOG_ERROR("Cannot append HLS playlist to a mapped download batch");
        return false;
    }
    for (const auto& playlist : playlists)
    {
        std::optional<std::size_t> queuedMap;
        for (const auto& segment : playlist.segments)
        {
            if (segment.mediaMapIndex && segment.mediaMapIndex != queuedMap)
            {
                const auto& map = playlist.mediaMaps[*segment.mediaMapIndex];
                const auto mapFilename = util::localeToUtf8((std::filesystem::path(m_dir) / ("map-" + std::to_string(m_taskFiles.size()) + ".mp4")).string());
                m_downloadInfos.push_back({map.getUri(), mapFilename, map.keyInfo, 0, map.hasByteRange ? map.byteRangeLength : 0, map.byteRangeOffset});
                m_taskFiles.push_back(mapFilename);
                m_hasMap = true;
                ++m_totalCount;
                queuedMap = segment.mediaMapIndex;
            }
            const auto filename =
                util::localeToUtf8((std::filesystem::path(m_dir) / (std::to_string(m_taskFiles.size()) + (playlist.hasMap ? ".m4s" : ".ts"))).string());
            const KeyInfo& key = segment.keyInfo.method == KeyInfo::None && segment.keyInfo.strMethod.empty() ? playlist.keyInfo : segment.keyInfo;
            m_downloadInfos.push_back({segment.getUri(), filename, key, segment.mediaSequence, segment.byteRangeLength, segment.byteRangeOffset});
            m_taskFiles.push_back(filename);
            ++m_totalCount;
        }
    }

    m_condition.notify_all();
    return true;
}

bool HLSPlaylistDownloader::addDownloadInfo(const std::string& uri, const std::string& filename, const KeyInfo& keyInfo)
{
    std::lock_guard lock(m_mutex);
    if (m_exit || uri.empty() || filename.empty() || std::find(m_taskFiles.begin(), m_taskFiles.end(), filename) != m_taskFiles.end())
    {
        HLS_LOG_ERROR("Cannot queue HLS download info: exited, missing URI or filename, or duplicate filename");
        return false;
    }

    m_downloadInfos.push_back({uri, filename, keyInfo, 0});
    m_taskFiles.push_back(filename);
    ++m_totalCount;

    m_condition.notify_one();
    return true;
}

bool HLSPlaylistDownloader::addDownloadInfo(const MediaSegment& segment, const KeyInfo& fallbackKey)
{
    std::lock_guard lock(m_mutex);
    if (m_exit)
    {
        HLS_LOG_ERROR("Cannot queue HLS segment after downloader exit");
        return false;
    }

    const auto filename = util::localeToUtf8((std::filesystem::path(dir()) / (std::to_string(m_taskFiles.size()) + ".ts")).string());
    if (segment.getUri().empty() || std::find(m_taskFiles.begin(), m_taskFiles.end(), filename) != m_taskFiles.end())
    {
        HLS_LOG_ERROR("Cannot queue HLS segment with an empty URI or duplicate filename");
        return false;
    }

    const KeyInfo& key = segment.keyInfo.method == KeyInfo::None && segment.keyInfo.strMethod.empty() ? fallbackKey : segment.keyInfo;
    m_downloadInfos.push_back({segment.getUri(), filename, key, segment.mediaSequence});
    m_taskFiles.push_back(filename);
    ++m_totalCount;

    m_condition.notify_one();
    return true;
}

int HLSPlaylistDownloader::totalCount() const
{
    return m_totalCount.load();
}

int HLSPlaylistDownloader::finishedCount() const
{
    return m_finishedCount.load();
}

int HLSPlaylistDownloader::failedCount() const
{
    return m_failedCount.load();
}

bool HLSPlaylistDownloader::mergeTs(const std::string& output)
{
    std::vector<std::string> files;
    bool hasMap = false;
    {
        std::lock_guard lock(m_mutex);
        files = m_taskFiles;
        hasMap = m_hasMap;
    }
    if (files.empty() || m_failedCount.load() != 0 || m_finishedCount.load() != m_totalCount.load())
    {
        HLS_LOG_ERROR("Cannot merge HLS segments: total {}, finished {}, failed {}, files {}", m_totalCount.load(), m_finishedCount.load(),
                      m_failedCount.load(), files.size());
        return false;
    }
    for (const auto& file : files)
    {
        std::error_code error;
        if (!std::filesystem::is_regular_file(util::utf8ToLocale(file), error) || std::filesystem::file_size(util::utf8ToLocale(file), error) == 0 || error)
        {
            HLS_LOG_ERROR("Cannot merge HLS segments: missing or empty input file: {}", file);
            return false;
        }
    }

    const auto intermediate = std::filesystem::path(dir()) / (hasMap ? "joined.mp4" : "concat.txt");
    const auto cleanup = [files, directory = std::filesystem::path(dir()), intermediate,
                          outputPath = std::filesystem::path(util::utf8ToLocale(output)).lexically_normal()] {
        for (const auto& file : files)
        {
            const auto path = std::filesystem::path(util::utf8ToLocale(file));
            if (path.lexically_normal() == outputPath)
            {
                continue;
            }
            std::error_code error;
            std::filesystem::remove(path, error);
            if (error)
            {
                HLS_LOG_ERROR("Unable to remove merged HLS input {}: {}", file, error.message());
            }
        }
        if (intermediate.lexically_normal() != outputPath)
        {
            std::error_code error;
            std::filesystem::remove(intermediate, error);
            if (error)
            {
                HLS_LOG_ERROR("Unable to remove HLS merge intermediate {}: {}", util::localeToUtf8(intermediate.string()), error.message());
            }
        }
        std::error_code error;
        std::filesystem::remove(directory, error);
        if (error && error != std::errc::directory_not_empty)
        {
            HLS_LOG_ERROR("Unable to remove HLS playlist directory {}: {}", util::localeToUtf8(directory.string()), error.message());
        }
    };

    if (hasMap)
    {
        const auto joinedPath = std::filesystem::path(dir()) / "joined.mp4";
        std::ofstream joined(joinedPath, std::ios::binary | std::ios::trunc);
        if (!joined)
        {
            HLS_LOG_ERROR("Unable to create HLS joined stream");
            return false;
        }
        for (const auto& file : files)
        {
            std::ifstream input(util::utf8ToLocale(file), std::ios::binary);
            if (!input)
            {
                HLS_LOG_ERROR("Unable to read HLS merge input: {}", file);
                return false;
            }
            joined << input.rdbuf();
            if (!joined || input.bad())
            {
                HLS_LOG_ERROR("Unable to append HLS merge input: {}", file);
                return false;
            }
        }
        joined.close();
        if (!joined.good())
        {
            HLS_LOG_ERROR("Unable to finalize HLS joined stream");
            return false;
        }
        return ffmpeg::FFmpegHelper().startFFmpeg(
            {"-i", util::localeToUtf8(joinedPath.string()), "-c", "copy", util::localeToUtf8(output), "-y"}, [] {}, cleanup);
    }
    return mergeTsFiles(dir(), files, output, cleanup);
}

void HLSPlaylistDownloader::startDownload()
{
    std::lock_guard lifecycleLock(m_lifecycleMutex);
    {
        std::lock_guard lock(m_mutex);
        if (m_exit)
        {
            HLS_LOG_ERROR("Cannot start HLS segment workers after exit");
            return;
        }
        m_resume = true;
    }
    if (m_downloadThreads.empty())
    {
        try
        {
            HLS_LOG_INFO("Starting {} HLS segment worker(s)", m_threadCount);
            m_downloadThreads.reserve(m_threadCount);
            for (int index = 0; index < m_threadCount; ++index)
            {
                m_downloadThreads.emplace_back(&HLSPlaylistDownloader::downloadTsFile, this);
            }
        }
        catch (...)
        {
            HLS_LOG_ERROR("Unable to start all HLS segment workers");
            const int remaining = totalCount() - finishedCount();
            m_failedCount = remaining > 0 ? remaining : 1;
        }
    }
    m_condition.notify_all();
}

void HLSPlaylistDownloader::stopDownload()
{
    {
        std::lock_guard lock(m_mutex);
        m_resume = false;
    }
    m_condition.notify_all();
}

void HLSPlaylistDownloader::exitDownload()
{
    std::lock_guard lifecycleLock(m_lifecycleMutex);
    {
        std::lock_guard lock(m_mutex);
        m_exit = true;
        m_resume = true;
        m_downloadInfos.clear();
    }
    m_condition.notify_all();
    for (auto& thread : m_downloadThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    m_downloadThreads.clear();
}

int HLSPlaylistDownloader::threadCount() const
{
    return m_threadCount;
}

void HLSPlaylistDownloader::setThreadCount(int count)
{
    std::lock_guard lifecycleLock(m_lifecycleMutex);
    if (count > 0 && m_downloadThreads.empty())
    {
        m_threadCount = count;
    }
}

void HLSPlaylistDownloader::downloadTsFile()
{
    HLS_LOG_INFO("HLS segment worker started");
    while (true)
    {
        DownloadInfo info;
        {
            std::unique_lock lock(m_mutex);
            m_condition.wait(lock, [&] {
                return m_exit || (m_resume && !m_downloadInfos.empty());
            });
            if (m_exit)
            {
                HLS_LOG_INFO("HLS segment worker exited");
                return;
            }
            info = std::move(m_downloadInfos.front());
            m_downloadInfos.pop_front();
        }
        try
        {
            if (m_client.downloadTsToFiles(info.uri, info.filename, info.keyInfo, info.mediaSequence, info.byteRangeLength, info.byteRangeOffset))
            {
                ++m_finishedCount;
            }
            else
            {
                HLS_LOG_ERROR("HLS segment download failed, output: {}", info.filename);
                ++m_failedCount;
            }
        }
        catch (const std::exception& error)
        {
            HLS_LOG_ERROR("HLS segment download failed, output: {}, error: {}", info.filename, error.what());
            ++m_failedCount;
        }
        catch (...)
        {
            HLS_LOG_ERROR("HLS segment download failed with an unknown exception, output: {}", info.filename);
            ++m_failedCount;
        }
    }
}
}  // namespace hlsapi
