#pragma once
#include <string>
#include <vector>
#include <list>
#include <thread>
#include <condition_variable>

#include "NetWork/CNetWork.h"
#include "NetWork/CurlCpp/CurlCookieJar.h"
#include "HLSApi.h"

namespace hlsapi
{

struct DownloadInfo
{
    std::string uri;
    std::string filename;
    KeyInfo keyInfo;
};

class HLSClient;
class HLSPlaylistDownloader
{
public:
    explicit HLSPlaylistDownloader(HLSClient& client, const std::string& dir);
    explicit HLSPlaylistDownloader(HLSClient& client);
    HLSPlaylistDownloader(const HLSPlaylistDownloader&) = delete;
    HLSPlaylistDownloader& operator=(const HLSPlaylistDownloader&) = delete;
    HLSPlaylistDownloader(HLSPlaylistDownloader&& other) = delete;
    HLSPlaylistDownloader& operator=(HLSPlaylistDownloader&& other) = delete;
    ~HLSPlaylistDownloader();

    HLSClient& client() const;

    void setDir(const std::string& dir);
    const std::string& dir() const;

    int totalCount() const;
    int finishedCount() const;

    bool downloadTsToFiles(const std::vector<M3U8Playlist>& playlist);
    bool addDownloadInfo(const std::string& uri, const std::string& filename, const KeyInfo& keyInfo = {});
    bool addDownloadInfo(const MediaSegment& mediaSegment, const KeyInfo& keyInfo = {});

    void mergeTs(const std::string& output);

    void startDownload();
    void stopDownload();
    void exitDownload();

protected:
    void downloadTsFile();

private:
    std::string m_dir;
    HLSClient& m_client;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::atomic_bool m_stop;
    std::atomic_bool m_resume{true};
    std::atomic_int m_totalCount{0};
    std::atomic_int m_finishedCount{0};
    std::list<DownloadInfo> m_downloadInfos;
    std::vector<std::thread> m_downloadThreads;
};

}  // namespace hlsapi
