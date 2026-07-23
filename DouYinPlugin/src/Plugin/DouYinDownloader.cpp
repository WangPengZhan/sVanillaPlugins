#include "DouYinDownloader.h"

#include <Download/FileDownloader.h>
#include <Download/AriaDownloader.h>
#include <FFmpeg/FFmpegHelper.h>
#include <Util/LocaleHelper.h>

namespace download
{

DouYinDownloader::DouYinDownloader()
    : m_finished(false)
    , m_haveTwoPart(false)
{
    m_videoDownloader.setStatus(Ready);
    m_audioDownloader.setStatus(Ready);
}

DouYinDownloader::DouYinDownloader(std::list<std::string> videoUris, std::list<std::string> audioUris, std::string path, std::string filename)
    : m_finished(false)
    , m_haveTwoPart(false)
    , m_path(std::move(path))
    , m_filename(std::move(filename))
    , m_videoDownloader(videoUris, m_path)
    , m_audioDownloader(audioUris, m_path)
{
    m_haveTwoPart = !videoUris.empty() && !audioUris.empty();
    setAriaFileName();
    m_videoDownloader.setStatus(Ready);
    m_audioDownloader.setStatus(Ready);
}

DouYinDownloader::DouYinDownloader(ResourceInfo info)
    : m_resourceInfo(std::move(info))
    , m_haveTwoPart(false)
    , m_path(m_resourceInfo.option.dir)
    , m_filename(m_resourceInfo.option.out)
    , m_videoDownloader(m_resourceInfo.videoUris, m_resourceInfo.option)
    , m_audioDownloader(m_resourceInfo.audioUris, m_resourceInfo.option)
    , m_finished(false)
{
    m_haveTwoPart = !m_resourceInfo.videoUris.empty() && !m_resourceInfo.audioUris.empty();
    setAriaFileName();
    m_videoDownloader.setStatus(Ready);
    m_audioDownloader.setStatus(Ready);
    m_uris = m_resourceInfo.videoUris;
}

void DouYinDownloader::start()
{
    m_videoDownloader.start();
    if (m_haveTwoPart)
    {
        m_audioDownloader.start();
    }
    m_info.stage = "audio/video downloading";
    m_status = Downloading;
}

void DouYinDownloader::stop()
{
    m_videoDownloader.stop();
    if (m_haveTwoPart)
    {
        m_audioDownloader.stop();
    }
    m_status = Stopped;
}

void DouYinDownloader::pause()
{
    m_videoDownloader.pause();
    if (m_haveTwoPart)
    {
        m_audioDownloader.pause();
    }
    m_status = Paused;
}

void DouYinDownloader::resume()
{
    m_videoDownloader.resume();
    if (m_haveTwoPart)
    {
        m_audioDownloader.resume();
    }
    m_status = Downloading;
}

void DouYinDownloader::downloadStatus()
{
    if (m_videoDownloader.status() == Error || (m_haveTwoPart && m_audioDownloader.status() == Error))
    {
        // DOWNLOAD_LOG_ERROR("DouYinDownloader downloadStatus error, filename: {}", filename());
        m_status = Error;
        return;
    }

    if (m_audioDownloader.status() == Downloading)
    {
        m_audioDownloader.downloadStatus();
    }

    if (m_videoDownloader.status() == Downloading)
    {
        m_videoDownloader.downloadStatus();
    }

    auto video = m_videoDownloader.info();
    auto audio = m_audioDownloader.info();

    m_info.total = video.total + audio.total;
    m_info.complete = video.complete + audio.complete;
    m_info.speed = video.speed + audio.speed;

    if (m_info.total == m_info.complete && m_videoDownloader.status() == Finished && (!m_haveTwoPart || m_audioDownloader.status() == Finished))
    {
        if (m_haveTwoPart)
        {
            ffmpeg::MergeInfo merge;
            merge.audio = m_audioDownloader.path() + "/" + m_audioDownloader.filename();
            merge.video = m_videoDownloader.path() + "/" + m_videoDownloader.filename();
            merge.targetVideo = path() + "/" + filename();
            m_info.stage = "ffmpeg merging";
            if (!ffmpeg::FFmpegHelper::mergeVideo(merge))
            {
                m_info.stage = "ffmpeg merge failed";
                m_status = Error;
                return;
            }
            m_info.stage = "ffmpeg merged";
        }

        m_status = Finished;
    }
}

void DouYinDownloader::finish()
{
    m_finished = true;
}

void DouYinDownloader::setVideoUris(const std::list<std::string>& videoUris)
{
    m_videoDownloader.setUris(videoUris);
}

void DouYinDownloader::setAudioUris(const std::list<std::string>& audioUri)
{
    m_audioDownloader.setUris(audioUri);
    m_haveTwoPart = !audioUri.empty();
}

void DouYinDownloader::setPath(std::string path)
{
    m_path = std::move(path);
    m_audioDownloader.setPath(m_path);
    m_videoDownloader.setPath(m_path);
}

const std::string& DouYinDownloader::path() const
{
    return m_path;
}

void DouYinDownloader::setFilename(std::string filename)
{
    m_filename = std::move(filename);
    setAriaFileName();
}

const std::string& DouYinDownloader::filename() const
{
    return m_filename;
}

const std::list<std::string>& DouYinDownloader::uris() const
{
    return m_uris;
}

bool DouYinDownloader::isFinished() const
{
    return m_finished;
}

std::string DouYinDownloader::nowDownload() const
{
    return std::string();
}

void DouYinDownloader::setAriaFileName()
{
    std::u8string u8BaseName = std::filesystem::path(util::utf8ToLocale(m_filename)).stem().u8string();
    std::string baseName = std::string(reinterpret_cast<const char*>(u8BaseName.data()), u8BaseName.size());
    m_haveTwoPart ? m_videoDownloader.setFilename(baseName + "_video.mp4") : m_videoDownloader.setFilename(m_filename);
    m_audioDownloader.setFilename(baseName + "_audio.mp3");
}

void freeDownloader(DouYinDownloader* downloader)
{
    if (downloader)
    {
        delete downloader;
    }
}

}  // namespace download
