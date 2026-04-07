#include "XHSDownloader.h"

#include <Download/FileDownloader.h>
#include <Download/AriaDownloader.h>

namespace download
{

XHSDownloader::XHSDownloader(const ResourceInfo& info)
    : FileDownloader()
{
}

void XHSDownloader::start()
{
    m_status = Status::Downloading;
    downloadVideo();
}

void XHSDownloader::stop()
{
    m_status = Status::Stopped;
}

void XHSDownloader::pause()
{
    m_status = Status::Paused;
}

void XHSDownloader::resume()
{
    m_status = Status::Downloading;
    downloadVideo();
}

void XHSDownloader::downloadStatus()
{
}

void XHSDownloader::finish()
{
}

void XHSDownloader::downloadVideo()
{
    if (m_status != Status::Downloading)
    {
        return;
    }
}

void XHSDownloader::setPath(std::string path)
{
    m_path = std::move(path);
}

const std::string& XHSDownloader::path() const
{
    return m_path;
}

void XHSDownloader::setFilename(std::string filename)
{
    m_filename = std::move(filename);
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
