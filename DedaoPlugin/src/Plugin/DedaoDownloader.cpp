#include <filesystem>
#include <utility>

#include "DedaoDownloader.h"

#include "FFmpeg/FFmpegHelper.h"
#include "Util/LocaleHelper.h"

namespace download
{

DedaoDownloader::DedaoDownloader()
{
    m_videoDownloader.setStatus(Ready);
    m_audioDownloader.setStatus(Ready);
}

DedaoDownloader::DedaoDownloader(std::list<std::string> videoUris, std::list<std::string> audioUris, std::string path, std::string filename)
    : m_path(std::move(path))
    , m_filename(std::move(filename))
    , m_uris(videoUris)
    , m_videoDownloader(videoUris, m_path)
    , m_audioDownloader(audioUris, m_path)
    , m_haveTwoPart(!videoUris.empty() && !audioUris.empty())
{
    setAriaFileName();
    m_videoDownloader.setStatus(Ready);
    m_audioDownloader.setStatus(Ready);
}

DedaoDownloader::DedaoDownloader(ResourceInfo info)
    : m_resourceInfo(std::move(info))
    , m_path(m_resourceInfo.option.dir)
    , m_filename(m_resourceInfo.option.out)
    , m_uris(m_resourceInfo.videoUris)
    , m_videoDownloader(m_resourceInfo.videoUris, m_resourceInfo.option)
    , m_audioDownloader(m_resourceInfo.audioUris, m_resourceInfo.option)
    , m_haveTwoPart(!m_resourceInfo.videoUris.empty() && !m_resourceInfo.audioUris.empty())
{
    setAriaFileName();
    m_videoDownloader.setStatus(Ready);
    m_audioDownloader.setStatus(Ready);
}

void DedaoDownloader::start()
{
    m_videoDownloader.start();
    if (m_haveTwoPart)
    {
        m_audioDownloader.start();
    }
    m_info.stage = "audio/video downloading";
    m_status = Downloading;
}

void DedaoDownloader::stop()
{
    m_videoDownloader.stop();
    if (m_haveTwoPart)
    {
        m_audioDownloader.stop();
    }
    m_status = Waiting;
}

void DedaoDownloader::pause()
{
    m_videoDownloader.pause();
    if (m_haveTwoPart)
    {
        m_audioDownloader.pause();
    }
    m_status = Paused;
}

void DedaoDownloader::resume()
{
    m_videoDownloader.resume();
    if (m_haveTwoPart)
    {
        m_audioDownloader.resume();
    }
    m_status = Downloading;
}

void DedaoDownloader::downloadStatus()
{
    // Terminal states must not be re-evaluated by repeated polls: a second
    // pass after Finished would run the FFmpeg merge again.
    if (m_status == Finished || m_status == Error)
    {
        return;
    }

    if (m_videoDownloader.status() == Error || (m_haveTwoPart && m_audioDownloader.status() == Error))
    {
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

    const auto video = m_videoDownloader.info();
    const auto audio = m_audioDownloader.info();

    m_info.total = video.total + audio.total;
    m_info.complete = video.complete + audio.complete;
    m_info.speed = video.speed + audio.speed;

    // total > 0 guards against a 0 == 0 comparison reporting success for an
    // empty response.
    if (m_info.total > 0 && m_info.total == m_info.complete && m_videoDownloader.status() == Finished &&
        (!m_haveTwoPart || m_audioDownloader.status() == Finished))
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

void DedaoDownloader::finish()
{
    m_finished = true;
}

void DedaoDownloader::setVideoUris(const std::list<std::string>& videoUris)
{
    m_uris = videoUris;
    m_videoDownloader.setUris(videoUris);
}

void DedaoDownloader::setAudioUris(const std::list<std::string>& audioUri)
{
    m_audioDownloader.setUris(audioUri);
    m_haveTwoPart = !audioUri.empty();
    setAriaFileName();
}

void DedaoDownloader::setPath(std::string path)
{
    m_path = std::move(path);
    m_audioDownloader.setPath(m_path);
    m_videoDownloader.setPath(m_path);
}

const std::string& DedaoDownloader::path() const
{
    return m_path;
}

void DedaoDownloader::setFilename(std::string filename)
{
    m_filename = std::move(filename);
    setAriaFileName();
}

const std::string& DedaoDownloader::filename() const
{
    return m_filename;
}

const std::list<std::string>& DedaoDownloader::uris() const
{
    return m_uris;
}

bool DedaoDownloader::isFinished() const
{
    return m_finished;
}

std::string DedaoDownloader::nowDownload() const
{
    return std::string();
}

void DedaoDownloader::setAriaFileName()
{
    std::u8string u8BaseName = std::filesystem::path(util::utf8ToLocale(m_filename)).stem().u8string();
    std::string baseName = std::string(reinterpret_cast<const char*>(u8BaseName.data()), u8BaseName.size());
    m_haveTwoPart ? m_videoDownloader.setFilename(baseName + "_video.mp4") : m_videoDownloader.setFilename(m_filename);
    m_audioDownloader.setFilename(baseName + "_audio.mp3");
}

void freeDownload(DedaoDownloader* downloader)
{
    if (downloader)
    {
        delete downloader;
    }
}

}  // namespace download
