#pragma once
#include <string>
#include <atomic>

#include <nlohmann/json.hpp>

#include "YoutubeApi.h"
#include "NetWork/CNetWork.h"
#include "NetWork/CurlCpp/CurlCookieJar.h"

namespace youtubeapi
{

class YoutubeClient : public network::NetWork
{
public:
    YoutubeClient();
    ~YoutubeClient() = default;

    static YoutubeClient& globalClient();

    [[nodiscard]] bool isLogined() const;
    void setCookie(const std::string& cookies);

    void setCookies(const std::string& cookies);
    std::string cookies() const;

    std::string getVisitorData();
    MainResponse getVideoInfo(const std::string& videoId);
    std::string getIFrameVersion();
    std::string getBaseJs(std::string version);
    std::vector<AdaptiveFormat> getStreamInfo(const std::string& videoId);

    bool logout();

    std::string visitorData();

    static nlohmann::json getDataFromRespones(const std::string& respones);

protected:
    void initDefaultHeaders();
    void initDefaultOptions();

private:
    network::CurlCookies m_cookies;  // we sellect this mode for debug
    std::string m_visitorData;
    std::mutex m_vistorDataMutex;
};

}  // namespace youtubeapi
