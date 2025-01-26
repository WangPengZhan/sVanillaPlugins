#pragma once
#include <string>

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
    void setCookies(const std::string& cookies);

    void getVideoInfo(const std::string& videoId);

    std::string getIFrameVersion();

    std::string getBaseJs(std::string version);

    bool logout();

    static nlohmann::json getDataFromRespones(const std::string& respones);

protected:
    void initDefaultHeaders();
    void initDefaultOptions();

private:
    network::CurlCookies m_cookies;  // we sellect this mode for debug
};

}  // namespace youtubeapi
