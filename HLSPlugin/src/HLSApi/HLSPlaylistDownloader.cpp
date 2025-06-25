#include <regex>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>
#include <future>

#include "Util/LocaleHelper.h"
#include "FFmpeg/FFmpegHelper.h"
#include "HLSPlaylistDownloader.h"
#include "HLSConstants.h"
#include "HLSClient.h"
#include "HLSLog.h"

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
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& playlist : playlists)
    {
        for (const auto& segment : playlist.segments)
        {
            DownloadInfo info;
            info.uri = segment.getUri();
            info.filename = util::localeToUtf8(dir() + std::to_string(m_downloadInfos.size()) + ".ts");
            info.keyInfo = playlist.keyInfo;
            m_downloadInfos.push_back(std::move(info));
        }
        m_totalCount += static_cast<int>(playlist.segments.size());
    }

    return true;
}

bool HLSPlaylistDownloader::addDownloadInfo(const std::string& uri, const std::string& filename, const KeyInfo& keyInfo)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    DownloadInfo info;
    info.uri = uri;
    info.filename = filename;
    info.keyInfo = keyInfo;
    m_downloadInfos.push_back(std::move(info));
    m_totalCount += 1;
    return true;
}

bool HLSPlaylistDownloader::addDownloadInfo(const MediaSegment& mediaSegment, const KeyInfo& keyInfo)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    DownloadInfo info;
    info.uri = mediaSegment.getUri();
    info.filename = util::localeToUtf8(dir() + std::to_string(m_downloadInfos.size()) + ".ts");
    info.keyInfo = keyInfo;
    m_downloadInfos.push_back(std::move(info));
    m_totalCount += 1;
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

void HLSPlaylistDownloader::mergeTs(const std::string& output)
{
    std::map<int, std::string> tsFiles;
    for (const auto& entry : std::filesystem::directory_iterator(dir()))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".ts")
        {
            auto name = entry.path().stem().string();
            int index = std::stoi(name);
            tsFiles[index] = entry.path().filename().string();
        }
    }

    std::vector<std::string> sortedFiles;
    for (const auto& pair : tsFiles)
    {
        sortedFiles.push_back(pair.second);
    }

    if (sortedFiles.empty())
    {
        HLS_LOG_ERROR("No TS files found in directory: {}", dir());
        return;
    }
    else if (sortedFiles.size() == 1)
    {
        HLS_LOG_INFO("Only one TS file found, no need to merge: {}", sortedFiles.front());
        std::filesystem::copy(std::filesystem::path(dir()) / sortedFiles.front(), output, std::filesystem::copy_options::overwrite_existing);
        HLS_LOG_INFO("Copied single TS file to output: {}", output);
        if (std::filesystem::exists(dir()))
        {
            std::filesystem::remove_all(dir());
        }
        return;
    }

    std::string concatFilePath = dir() + "concat.txt";
    std::ofstream concatFile(concatFilePath);
    for (const auto& file : sortedFiles)
    {
        concatFile << "file \'" << file << "\'\n";
    }
    concatFile.close();
    HLS_LOG_INFO("Created concat file: {}", concatFilePath);

    ffmpeg::MergeTsInfo mergeInfo;
    mergeInfo.concatFile = util::localeToUtf8(concatFilePath);
    mergeInfo.targetVideo = util::localeToUtf8(output);
    ffmpeg::FFmpegHelper().startFFmpeg(
        mergeInfo,
        [mergeInfo, sortedFiles]() {
            std::string fileList;
            for (const auto& file : sortedFiles)
            {
                fileList += file + "\n";
            }
            HLS_LOG_ERROR("Failed to merge TS files: {}, with files: {}", mergeInfo.targetVideo, fileList);
        },
        [mergeInfo, dir = dir()]() {
            if (std::filesystem::exists(dir))
            {
                std::filesystem::remove_all(dir);
            }
        });
}

void HLSPlaylistDownloader::startDownload()
{
    if (m_downloadThreads.empty())
    {
        std::vector<std::thread> threads;
        threads.reserve(5);
        for (int i = 0; i < 5; ++i)
        {
            threads.emplace_back(std::thread(&HLSPlaylistDownloader::downloadTsFile, this));
        }
        m_downloadThreads = std::move(threads);
    }

    m_stop = false;
    m_resume = true;
    m_condition.notify_all();
}

void HLSPlaylistDownloader::stopDownload()
{
    m_resume = false;
    m_condition.notify_all();
}

void HLSPlaylistDownloader::exitDownload()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_downloadInfos.clear();
    }
    m_stop = true;
    m_resume = true;
    m_condition.notify_all();
    for (auto& thread : m_downloadThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void HLSPlaylistDownloader::downloadTsFile()
{
    HLS_LOG_INFO("Downloaded TS file start, task size: {}", m_downloadInfos.size());
    while (true)
    {
        DownloadInfo downloadInfo;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [&] {
                return (m_stop || !m_downloadInfos.empty()) && m_resume;
            });
            if (m_stop && m_downloadInfos.empty())
            {
                HLS_LOG_INFO("Downloaded TS file exit!");
                return;
            }
            downloadInfo = std::move(m_downloadInfos.front());
            m_downloadInfos.pop_front();
        }
        m_client.downloadTsToFiles(downloadInfo.uri, downloadInfo.filename, downloadInfo.keyInfo);
        m_finishedCount++;
        HLS_LOG_INFO("Downloaded TS file: {}, total finished: {}", downloadInfo.filename, m_finishedCount.load());
    }
}

}  // namespace hlsapi
