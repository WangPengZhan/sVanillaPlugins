#pragma once
#include <string>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "HLSApi/HLSApi.h"
#include "HLSApi/HLSPlaylistDownloader.h"

#include <Download/FileDownloader.h>
#include <Download/AriaDownloader.h>

namespace hlsapi
{
class HLSClient;
}

namespace download
{

class HLSDownloader : public FileDownloader
{
public:
    HLSDownloader();
    explicit HLSDownloader(std::shared_ptr<hlsapi::HLSClient> client, std::unordered_map<hlsapi::MediaInfo::MediaType, hlsapi::M3U8Playlist> downloadedFiles);
    ~HLSDownloader() = default;

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

    bool isFinished() const;

    std::string nowDownload() const;

    void addDownloadFile(hlsapi::MediaInfo::MediaType type, hlsapi::M3U8Playlist playlist);

private:
    std::string m_path;
    std::string m_filename;
    std::string m_uniqueId;
    std::string m_tempDir;
    std::shared_ptr<hlsapi::HLSClient> m_client;
    bool m_finished;
    std::unordered_map<hlsapi::MediaInfo::MediaType, std::shared_ptr<hlsapi::HLSPlaylistDownloader>> m_downloadedFiles;
    std::unordered_set<hlsapi::MediaInfo::MediaType> m_mergedType;
};

void freeDownload(HLSDownloader* downloader);

}  // namespace download
