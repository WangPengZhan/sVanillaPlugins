#include "XHSDownloader.h"

#include <Download/FileDownloader.h>
#include <Download/AriaDownloader.h>

namespace download
{

XHSDownloader::XHSDownloader(const ResourceInfo& info)
    : m_resourceInfo(info)
    , m_videoDownloader(m_resourceInfo.videoUris, m_resourceInfo.option)
    , m_path(m_resourceInfo.option.dir)
    , m_filename(m_resourceInfo.option.out)
{
    m_videoDownloader.setStatus(Ready);
    m_status = Ready;
}

void XHSDownloader::start()
{
    if (m_resourceInfo.videoUris.empty())
    {
        m_status = Status::Error;
        return;
    }
    m_videoDownloader.start();
    m_status = m_videoDownloader.status();
    m_info.stage = "video downloading";
}

void XHSDownloader::stop()
{
    m_videoDownloader.stop();
    m_status = Status::Stopped;
}

void XHSDownloader::pause()
{
    m_videoDownloader.pause();
    m_status = m_videoDownloader.status();
}

void XHSDownloader::resume()
{
    m_videoDownloader.resume();
    m_status = m_videoDownloader.status();
}

void XHSDownloader::downloadStatus()
{
    if (m_videoDownloader.status() == Status::Downloading)
    {
        m_videoDownloader.downloadStatus();
    }
    m_info = m_videoDownloader.info();
    m_status = m_videoDownloader.status();
}

void XHSDownloader::finish()
{
    m_videoDownloader.finish();
}

void XHSDownloader::setPath(std::string path)
{
    m_path = std::move(path);
    m_videoDownloader.setPath(m_path);
}

const std::string& XHSDownloader::path() const
{
    return m_path;
}

void XHSDownloader::setFilename(std::string filename)
{
    m_filename = std::move(filename);
    m_videoDownloader.setFilename(m_filename);
}

const std::string& XHSDownloader::filename() const
{
    return m_filename;
}

void freeDownloader(XHSDownloader* downloader)
{
    if (downloader)
    {
        delete downloader;
    }
}

}  // namespace download
