#include <utility>

#include "YoutubeDownloader.h"
#include "FFmpeg/FFmpegHelper.h"
#include "Util/LocaleHelper.h"

namespace download
{
YoutubeDownloader::YoutubeDownloader()
    : m_finished(false)
    , m_haveTwoPart(false)
{
    m_videoDownloader.setStatus(Ready);
    m_audioDownloader.setStatus(Ready);
}

YoutubeDownloader::YoutubeDownloader(std::list<std::string> videoUris, std::list<std::string> audioUris, std::string path, std::string filename)
    : m_finished(false)
    , m_haveTwoPart(false)
    , m_path(std::move(path))
    , m_filename(std::move(filename))
    , m_videoDownloader(videoUris, m_path)
    , m_audioDownloader(audioUris, m_path)
{
    m_haveTwoPart = !videoUris.empty() && audioUris.empty();
    setAriaFileName();
    m_videoDownloader.setStatus(Ready);
    m_audioDownloader.setStatus(Ready);
    m_haveTwoPart = !videoUris.empty() && !audioUris.empty();
}

YoutubeDownloader::YoutubeDownloader(ResourseInfo info)
    : m_resourseInfo(std::move(info))
    , m_haveTwoPart(false)
    , m_path(m_resourseInfo.option.dir)
    , m_filename(m_resourseInfo.option.out)
    , m_videoDownloader(m_resourseInfo.videoUris, m_resourseInfo.option)
    , m_audioDownloader(m_resourseInfo.audioUris, m_resourseInfo.option)
    , m_finished(false)
{
    m_haveTwoPart = !m_resourseInfo.videoUris.empty() && !m_resourseInfo.audioUris.empty();
    setAriaFileName();
    m_videoDownloader.setStatus(Ready);
    m_audioDownloader.setStatus(Ready);
    m_uris = m_resourseInfo.videoUris;
}

void YoutubeDownloader::start()
{
    m_videoDownloader.start();
    if (m_haveTwoPart)
    {
        m_audioDownloader.start();
    }
    m_info.stage = "audio/video downloading";
    m_status = Downloading;
}

void YoutubeDownloader::stop()
{
    m_videoDownloader.stop();
    if (m_haveTwoPart)
    {
        m_audioDownloader.stop();
    }
    m_status = Waitting;
}

void YoutubeDownloader::pause()
{
    m_videoDownloader.pause();
    if (m_haveTwoPart)
    {
        m_audioDownloader.pause();
    }
    m_status = Paused;
}

void YoutubeDownloader::resume()
{
    m_videoDownloader.resume();
    if (m_haveTwoPart)
    {
        m_audioDownloader.resume();
    }
    m_status = Downloading;
}

void YoutubeDownloader::downloadStatus()
{
    if (m_videoDownloader.status() == Error || (m_haveTwoPart && m_audioDownloader.status() == Error))
    {
        // DOWNLOAD_LOG_ERROR("YoutubeDownloader downloadStatus error, filename: {}", filename());
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
            m_info.stage = "ffmpeg mixed!";

            ffmpeg::FFmpegHelper::mergeVideo(merge);
        }

        m_status = Finished;
    }
}

void YoutubeDownloader::finish()
{
    m_finished = true;
}

void YoutubeDownloader::setVideoUris(const std::list<std::string>& videoUris)
{
    m_videoDownloader.setUris(videoUris);
}

void YoutubeDownloader::setAudioUris(const std::list<std::string>& audioUri)
{
    m_audioDownloader.setUris(audioUri);
    m_haveTwoPart = !audioUri.empty();
}

void YoutubeDownloader::setPath(std::string path)
{
    m_path = std::move(path);
    m_audioDownloader.setPath(m_path);
    m_videoDownloader.setPath(m_path);
}

const std::string& YoutubeDownloader::path() const
{
    return m_path;
}

void YoutubeDownloader::setFilename(std::string filename)
{
    m_filename = std::move(filename);
    setAriaFileName();
}

const std::string& YoutubeDownloader::filename() const
{
    return m_filename;
}

const std::list<std::string>& YoutubeDownloader::uris() const
{
    return m_uris;
}

bool YoutubeDownloader::isFinished() const
{
    return m_finished;
}

std::string YoutubeDownloader::nowDownload() const
{
    return std::string();
}

void YoutubeDownloader::setAriaFileName()
{
    std::u8string u8BaseName = std::filesystem::path(util::utf8ToLocale(m_filename)).stem().u8string();
    std::string baseName = std::string(reinterpret_cast<const char*>(u8BaseName.data()), u8BaseName.size());
    m_haveTwoPart ? m_videoDownloader.setFilename(baseName + "_video.mp4") : m_videoDownloader.setFilename(m_filename);
    m_audioDownloader.setFilename(baseName + "_audio.mp3");
}

void freeDownload(YoutubeDownloader* downloader)
{
    if (downloader)
    {
        delete downloader;
    }
}

}  // namespace download
