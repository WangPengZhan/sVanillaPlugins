#include <utility>

#include "HLSDownloader.h"
#include "FFmpeg/FFmpegHelper.h"
#include "Util/LocaleHelper.h"
#include "Plugin/HLSPlugin.h"
#include "HLSApi/HLSClient.h"
#include "HLSApi/HLSLog.h"

namespace download
{
HLSDownloader::HLSDownloader()
    : m_finished(false)
    , m_tempDir(HLSPlugin::getDir() + "HLS_" + std::to_string(std::rand() % 10000) + "/")
{
    if (!std::filesystem::exists(std::filesystem::path(m_tempDir)))
    {
        std::filesystem::create_directories(std::filesystem::path(m_tempDir));
    }
}

HLSDownloader::HLSDownloader(std::shared_ptr<hlsapi::HLSClient> client, std::unordered_map<hlsapi::MediaInfo::MediaType, hlsapi::M3U8Playlist> downloadedFiles)
    : m_finished(false)
    , m_client(std::move(client))
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    m_tempDir = HLSPlugin::getDir() + "HLS_" + std::to_string(std::rand() % 10000) + "_" + std::to_string(std::time(nullptr)) + "/";
    if (!std::filesystem::exists(std::filesystem::path(m_tempDir)))
    {
        std::filesystem::create_directories(std::filesystem::path(m_tempDir));
    }

    for (auto& [type, playlist] : downloadedFiles)
    {
        m_downloadedFiles[type] = std::make_shared<hlsapi::HLSPlaylistDownloader>(*m_client);
        m_downloadedFiles[type]->setDir(m_tempDir + hlsapi::MediaInfo::mediaTypeToString(type) + "/");
        m_downloadedFiles[type]->downloadTsToFiles({playlist});
    }
}

void HLSDownloader::start()
{
    m_info.stage = "audio/video downloading";
    for (auto& [type, downloader] : m_downloadedFiles)
    {
        if (downloader)
        {
            downloader->startDownload();
        }
    }
    m_status = Downloading;
}

void HLSDownloader::stop()
{
    for (auto& [type, downloader] : m_downloadedFiles)
    {
        if (downloader)
        {
            downloader->stopDownload();
        }
    }
    m_status = Waitting;
}

void HLSDownloader::pause()
{
    for (auto& [type, downloader] : m_downloadedFiles)
    {
        if (downloader)
        {
            downloader->stopDownload();
        }
    }
    m_status = Paused;
}

void HLSDownloader::resume()
{
    for (auto& [type, downloader] : m_downloadedFiles)
    {
        if (downloader)
        {
            downloader->startDownload();
        }
    }
    m_status = Downloading;
}

void HLSDownloader::downloadStatus()
{
    m_info.total = 0;
    m_info.complete = 0;
    for (auto& [type, downloader] : m_downloadedFiles)
    {
        if (downloader)
        {
            m_info.total += downloader->totalCount();
            m_info.complete += downloader->finishedCount();
            m_info.speed += 10 * 1024;

            if (m_mergedType.find(type) != m_mergedType.end())
            {
                continue;
            }

            if (downloader->finishedCount() == downloader->totalCount())
            {
                switch (type)
                {
                case hlsapi::MediaInfo::Video:
                {
                    downloader->mergeTs(m_tempDir + "video.mp4");
                    break;
                }
                case hlsapi::MediaInfo::Audio:
                {
                    downloader->mergeTs(m_tempDir + "audio.m4a");
                    break;
                }
                case hlsapi::MediaInfo::Subtitle:
                {
                    downloader->mergeTs(m_tempDir + "subtitle.srt");
                    break;
                }
                case hlsapi::MediaInfo::CloseCaption:
                default:
                    break;
                }
                m_mergedType.insert(type);
            }
        }
    }

    for (const auto& [type, downloader] : m_downloadedFiles)
    {
        if (downloader && downloader->finishedCount() != downloader->totalCount())
        {
            return;
        }
    }

    ffmpeg::MergeInfo merge;
    merge.targetVideo = path() + "/" + filename();
    m_info.stage = "ffmpeg mixed!";

    for (auto& [type, downloader] : m_downloadedFiles)
    {
        if (downloader && downloader->totalCount() > 0)
        {
            switch (type)
            {
            case hlsapi::MediaInfo::Video:
                merge.video = util::localeToUtf8(m_tempDir) + "video.mp4";
                break;
            case hlsapi::MediaInfo::Audio:
                merge.audio = util::localeToUtf8(m_tempDir) + "audio.m4a";
                break;
            case hlsapi::MediaInfo::Subtitle:
                merge.subtitle = util::localeToUtf8(m_tempDir) + "subtitle.srt";
                break;
            default:
                break;
            }
        }
    }

    ffmpeg::FFmpegHelper::mergeVideo(merge);
    m_status = Finished;
    HLS_LOG_INFO("HLS download finished, merged video: {}", merge.targetVideo);
}

void HLSDownloader::finish()
{
    std::filesystem::remove_all(m_tempDir);
    m_finished = true;
}

void HLSDownloader::setPath(std::string path)
{
    std::filesystem::path fsPath = util::utf8ToLocale(path);
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
    return std::string();
}

void HLSDownloader::addDownloadFile(hlsapi::MediaInfo::MediaType type, hlsapi::M3U8Playlist playlist)
{
    if (m_downloadedFiles.find(type) == m_downloadedFiles.end())
    {
        if (m_downloadedFiles.empty())
        {
            m_downloadedFiles[type] =
                std::make_shared<hlsapi::HLSPlaylistDownloader>(hlsapi::HLSClient::globalClient(), m_tempDir + hlsapi::MediaInfo::mediaTypeToString(type));
        }
        else
        {
            m_downloadedFiles[type] = std::make_shared<hlsapi::HLSPlaylistDownloader>(m_downloadedFiles.begin()->second->client(),
                                                                                      m_tempDir + hlsapi::MediaInfo::mediaTypeToString(type) + "/");
        }
    }

    m_downloadedFiles[type]->downloadTsToFiles({playlist});
}

void freeDownload(HLSDownloader* downloader)
{
    if (downloader)
    {
        delete downloader;
    }
}

}  // namespace download
