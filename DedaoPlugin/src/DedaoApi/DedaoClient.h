#pragma once
#include <string>

#include "DedaoApi.h"
#include "NetWork/CNetWork.h"
#include "NetWork/CurlCpp/CurlCookieJar.h"

namespace dedaoapi
{

class DedaoClient : public network::NetWork
{
public:
    DedaoClient();
    ~DedaoClient() = default;

    static DedaoClient& globalClient();

    [[nodiscard]] bool isLogined() const;
    std::string cookie() const;

    std::string cookies() const;
    void setCookies(std::string cookies);

    std::string csrfToken();
    std::string loginToken();
    LoginQrcode loginQrcode(const std::string& token);
    LoginCheck loginCheck(const std::string& token, const std::string& qrCodeString);
    V2TokenInfo getV2Token();

    UserInfoResponse userInfo();

    LiveInfo liveInfo(const std::string& alias_id);
    LiveTokenInfo livetoken(const std::string& alias_id, const std::string& invite_code = "");
    RoomDetail roomDetail(const std::string& alias_id, const std::string& token);
    MediaStreamInfo webStreamInfo(const std::string& alias_id, const std::string& media_id_str, const std::string& security_token,
                                  const std::string& media_alias_id = "");

    CourseInfo courseInfo(const std::string& detail_id);
    ArticleInfo articleInfo(const std::string& detail_id, int count, bool reverse, int max_order_num, bool unlearn_switch, const std::string& chapter_id);

    bool logout(const std::string& token);

    static void parseCookie(const std::string& url);
    static nlohmann::json getDataFromRespones(const std::string& respones);

protected:
    void initDefaultHeaders();
    void initDefaultOptions();
    network::CurlHeader createLiveHeader(const std::string& alias_id);
    network::CurlHeader createCourseHeader(const std::string& detail_id);

private:
    network::CurlCookies m_cookies;  // we sellect this mode for debug
};

}  // namespace dedaoapi
