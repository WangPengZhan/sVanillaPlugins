#pragma once
#include <string>
#include <list>

#include <Download/FileDownloader.h>
#include <Download/AriaDownloader.h>

namespace download
{

class YoutubeDownloader : public FileDownloader
{
public:
    YoutubeDownloader();
    YoutubeDownloader(std::list<std::string> videoUris, std::list<std::string> audioUri = {}, std::string path = "", std::string filename = "");
    explicit YoutubeDownloader(ResourceInfo info);
    ~YoutubeDownloader() = default;

    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    void downloadStatus() override;
    void finish() override;

    void setVideoUris(const std::list<std::string>& videoUris);
    void setAudioUris(const std::list<std::string>& audioUri);
    void setPath(std::string path) override;
    const std::string& path() const override;
    void setFilename(std::string filename) override;
    const std::string& filename() const override;
    const std::list<std::string>& uris() const;

    bool isFinished() const;

    std::string nowDownload() const;

private:
    void setAriaFileName();

private:
    ResourceInfo m_resourceInfo;
    std::string m_path;
    std::string m_filename;
    std::string m_uniqueId;
    std::list<std::string> m_uris;
    AriaDownloader m_videoDownloader;
    AriaDownloader m_audioDownloader;
    bool m_finished;
    bool m_haveTwoPart;
};

void freeDownload(YoutubeDownloader* downloader);

}  // namespace download
