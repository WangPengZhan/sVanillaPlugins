#pragma once
#include <Download/FileDownloader.h>

namespace download
{

class XHSDownloader : public FileDownloader
{
public:
    explicit XHSDownloader(const ResourceInfo& info);
    ~XHSDownloader() override = default;

    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;

    void downloadStatus() override;
    void finish() override;

    void setPath(std::string path) override;
    const std::string& path() const override;
    void setFilename(std::string filename) override;
    const std::string& filename() const override;

private:
    void downloadVideo();

private:
    std::string m_path;
    std::string m_filename;
};

void freeDownloader(XHSDownloader* downloader);

}  // namespace download
