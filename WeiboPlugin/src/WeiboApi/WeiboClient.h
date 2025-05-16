#pragma once
#include <string>

#include "WeiBoApi.h"
#include "NetWork/CNetWork.h"
#include "NetWork/CurlCpp/CurlCookieJar.h"

namespace weiboapi
{

class WeiboClient : public network::NetWork
{
public:
    WeiboClient();
    ~WeiboClient() = default;

    static WeiboClient& globalClient();

    ComponentPlayPlayinfoResponse getPlayInfoByMid(const std::string& mid);
    WeiboAjaxData getPlayInfoByWid(const std::string& wid);

    UserMblogResponse getUserMlog(const std::string& uid);

    std::string getStreamInfo(const std::string& wid);

    QRCResponse getLoginUrl();
    bool getQrcImage(const std::string& url, const std::filesystem::path& path);
    LoginScaningStatus getLoginStatus(const std::string& qrid);
    LoginInfo loginWeibo(const std::string alt);
    bool crossDomainRequest(std::vector<std::string> urls);
    [[nodiscard]] bool isLogined() const;
    std::string cookie() const;

    void getDetailInfo(const std::string& mid);

    static void parseCookie(const std::string& url);
    static nlohmann::json getDataFromPJsonRespones(const std::string& respones);
    static nlohmann::json getDataFromRespones(const std::string& respones);

    static network::CurlHeader passPortHeaders();

protected:
    void initDefaultHeaders();
    void initDefaultOptions();

private:
    network::CurlCookies m_cookies;  // we sellect this mode for debug
};

}  // namespace weiboapi
