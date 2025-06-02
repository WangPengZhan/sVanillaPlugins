#pragma once
#include <string>

#include "NetWork/CNetWork.h"
#include "NetWork/CurlCpp/CurlCookieJar.h"
#include "HLSApi.h"

namespace hlsapi
{

class HLSClient : public network::NetWork
{
public:
    HLSClient();
    ~HLSClient() = default;

    static HLSClient& globalClient();

    std::string downloadM3u8(const std::string& url);

    bool downloadTsToFiles(const std::string& uri, const std::string& filename, const KeyInfo& keyInfo = {});

    void mergeTs(const std::string& dir, const std::string& output);

protected:
    void initDefaultHeaders();
    void initDefaultOptions();

private:
    network::CurlCookies m_cookies;  // we sellect this mode for debug
    std::vector<std::string> m_urls;
};

}  // namespace hlsapi
