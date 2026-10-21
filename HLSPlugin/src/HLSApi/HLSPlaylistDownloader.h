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
    std::uint64_t mediaSequence{};
    std::size_t byteRangeLength{};
    std::size_t byteRangeOffset{};
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
    int failedCount() const;

    bool downloadTsToFiles(const std::vector<M3U8Playlist>& playlist);
    bool addDownloadInfo(const std::string& uri, const std::string& filename, const KeyInfo& keyInfo = {});
    bool addDownloadInfo(const MediaSegment& mediaSegment, const KeyInfo& keyInfo = {});

    bool mergeTs(const std::string& output);

    void startDownload();
    void stopDownload();
    void exitDownload();

    int threadCount() const;
    void setThreadCount(int count);

protected:
    void downloadTsFile();

private:
    std::string m_dir;
    HLSClient& m_client;

    mutable std::mutex m_mutex;
    std::mutex m_lifecycleMutex;

    std::condition_variable m_condition;
    bool m_exit{};
    bool m_resume{true};

    std::atomic_int m_totalCount{0};
    std::atomic_int m_finishedCount{0};
    std::atomic_int m_failedCount{0};

    std::list<DownloadInfo> m_downloadInfos;
    std::vector<std::string> m_taskFiles;
    bool m_hasMap{};

    int m_threadCount{5};
    std::vector<std::thread> m_downloadThreads;
};

}  // namespace hlsapi
