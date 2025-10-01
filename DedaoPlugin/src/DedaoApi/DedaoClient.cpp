#include <regex>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>

#include "DedaoClient.h"
#include "DedaoApiConstants.h"
#include "Util/JsonProcess.h"
#include "DedaoLog.h"
#include "NetWork/HeaderBodyResponseWrapper.h"
#include "NetWork/CurlCpp/CurlCookie.h"
#include "NetWork/CurlCpp/CurlCookieOpt.h"

namespace dedaoapi
{

DedaoClient::DedaoClient()
{
    initDefaultHeaders();
    initDefaultOptions();
}

DedaoClient& DedaoClient::globalClient()
{
    static DedaoClient dedaoClient;
    return dedaoClient;
}

bool DedaoClient::isLogined() const
{
    return m_cookies.cookie(domain).contains("GAT");
}

std::string DedaoClient::cookie() const
{
    return network::CurlCookieOpt(m_cookies.cookie(domain)).shortContent();
}

std::string DedaoClient::cookies() const
{
    return std::string(m_cookies);
}

void DedaoClient::setCookies(std::string cookies)
{
    std::lock_guard lk(m_mutexRequest);
    m_cookies = network::CurlCookies(cookies);
    m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
}

std::string DedaoClient::csrfToken()
{
    network::CurlCookie cookie;
    {
        std::shared_lock lk(m_mutexRequest);
        cookie = m_cookies.cookie(domain);
    }

    if (cookie.contains("csrfToken"))
    {
        return cookie.value("csrfToken");
    }

    network::CurlHeader headers;
    headers.add("Cache-Control: no-cache");
    headers.add("Connection: keep-alive");
    headers.add(std::string("Referer:") + dedaoHomeUrl);
    network::ResponseHeaderAndBody response;
    get(dedaoHomeUrl, response, headers, true);
    auto header = parseHeader(response.header);
    if (header.end() != header.find(network::set_cookies))
    {
        std::lock_guard lk(m_mutexRequest);
        m_cookies.addCurlCookie(header.at(network::set_cookies));
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
        cookie = m_cookies.cookie(domain);
        if (cookie.contains("csrfToken"))
        {
            return cookie.value("csrfToken");
        }
    }

    return "";
}

std::string DedaoClient::loginToken()
{
    network::CurlHeader headers;
    headers.add("xi-csrf-token:" + csrfToken());
    headers.add(std::string("Referer:") + dedaoHomeUrl);
    headers.add("origin: https://www.dedao.cn");
    headers.add("accept: application/json, text/plain, */*");

    std::string response;
    post(dedaoLoginToken, response, headers, true);

    return response;
}

LoginQrcode DedaoClient::loginQrcode(const std::string& token)
{
    network::CurlHeader headers;
    headers.add("Content-Type: application/json");
    headers.add("x-oauth-access-token: " + token);
    headers.add(std::string("Referer:") + dedaoHomeUrl);

    std::string response;
    get(dedaoQrcodeUrl, response, headers, true);

    LoginQrcode loginQrcode;
    try
    {
        loginQrcode = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
    }

    return loginQrcode;
}

LoginCheck DedaoClient::loginCheck(const std::string& token, const std::string& qrCodeString)
{
    network::CurlHeader headers;
    headers.add("Content-Type: application/json");
    headers.add("x-oauth-access-token: " + token);
    headers.add(std::string("Referer:") + dedaoHomeUrl);

    nlohmann::json content = nlohmann::json::object();
    content["keepLogin"] = true;
    content["pname"] = "mobilesms";
    content["qrCode"] = qrCodeString;
    content["scene"] = "login";
    std::string param = content.dump();

    network::ResponseHeaderAndBody response;
    post(dedaoQrcodeCheckUrl, response, param, headers, true);

    LoginCheck ret;
    try
    {
        ret = getDataFromRespones(response.body);
    }
    catch (const std::exception& e)
    {
    }

    // add cookie
    if (ret.data.status == 1)
    {
        auto header = parseHeader(response.header);
        if (header.end() != header.find(network::set_cookies))
        {
            std::lock_guard lk(m_mutexRequest);
            m_cookies.addCurlCookie(header.at(network::set_cookies));
            m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
        }
    }

    return ret;
}

V2TokenInfo DedaoClient::getV2Token()
{
    network::CurlHeader headers;
    headers.add("xi-csrf-token:" + csrfToken());
    headers.add("Content-Type: application/json");
    headers.add(std::string("referer: " + dedaoHomeUrl));
    headers.add("xi-csrf-token:" + csrfToken());
    headers.add("xi-dt: web");

    std::string response;
    get(dedaoV2Token, response, headers, true);

    V2TokenInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
    }

    return ret;
}

UserInfoResponse DedaoClient::userInfo()
{
    network::CurlHeader headers;
    headers.add("xi-csrf-token:" + csrfToken());
    headers.add("Content-Type: application/json");

    std::string response;
    get(userInfoUrl, response, headers, true);

    UserInfoResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
    }

    return ret;
}

LiveInfo DedaoClient::liveInfo(const std::string& alias_id)
{
    nlohmann::json data;
    data["alias_id"] = alias_id;
    auto param = data.dump();

    std::string response;
    post(dedaoLiveBaseUrl, response, param, createLiveHeader(alias_id), true);

    LiveInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
    }

    return ret;
}

LiveTokenInfo DedaoClient::livetoken(const std::string& alias_id, const std::string& invite_code)
{
    nlohmann::json data;
    data["alias_id"] = alias_id;
    data["invite_code"] = invite_code;
    auto param = data.dump();

    std::string response;
    post(dedaoLiveCheckUrl, response, param, createLiveHeader(alias_id), true);

    LiveTokenInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
    }

    return ret;
}

RoomDetail DedaoClient::roomDetail(const std::string& alias_id, const std::string& token)
{
    nlohmann::json data;
    data["alias_id"] = alias_id;
    data["live_user_union_id"] = "";
    data["token"] = token;
    auto param = data.dump();

    std::string response;
    post(dedaoLiveRoomUrl, response, param, createLiveHeader(alias_id), true);

    RoomDetail ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
    }

    return ret;
}

MediaStreamInfo DedaoClient::webStreamInfo(const std::string& alias_id, const std::string& media_id_str, const std::string& security_token,
                                           const std::string& media_alias_id)
{
    nlohmann::json data;
    data["media_alias_id"] = media_alias_id;
    data["media_id_str"] = media_id_str;
    data["security_token"] = security_token;
    auto param = data.dump();

    std::string response;
    post(dedaoMediaWebUrl, response, param, createLiveHeader(alias_id), true);

    MediaStreamInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
    }

    return ret;
}

CourseInfo DedaoClient::courseInfo(const std::string& detail_id)
{
    nlohmann::json data;
    data["detail_id"] = detail_id;
    data["is_login"] = 1;
    auto param = data.dump();

    std::string response;
    post(dedaoClassUrl, response, param, createCourseHeader(detail_id), true);

    CourseInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
    }

    return ret;
}

ArticleInfo DedaoClient::articleInfo(const std::string& detail_id, int count, bool reverse, int max_id, bool unlearn_switch, const std::string& chapter_id)
{
    nlohmann::json data;
    data["chapter_id"] = chapter_id;
    data["count"] = count;
    data["detail_id"] = detail_id;
    data["include_edge"] = false;
    data["is_unlearn"] = false;
    data["max_id"] = max_id;
    data["max_order_num"] = 0;
    data["reverse"] = reverse;
    data["since_id"] = 0;
    data["since_order_num"] = 0;
    data["unlearn_switch"] = unlearn_switch;
    auto param = data.dump();

    std::string response;
    post(dedaoActicleList, response, param, createCourseHeader(detail_id), true);

    ArticleInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
    }

    return ret;
}

bool DedaoClient::logout(const std::string& token)
{
    nlohmann::json data;
    data["token"] = token;
    auto param = data.dump();

    network::CurlHeader headers;
    headers.add(std::string("referer: ") + dedaoHomeUrl);
    headers.add("xi-csrf-token:" + csrfToken());
    headers.add("xi-dt: web");

    std::string response;
    post(dedaoLogoutUrl, response, param, headers, true);

    return true;
}

void DedaoClient::parseCookie(const std::string& url)
{
}

nlohmann::json DedaoClient::getDataFromRespones(const std::string& respones)
{
    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(respones);
        util::JsonProcess::removeNullValues(json);
    }
    catch (std::exception& e)
    {
        DEDAO_LOG_ERROR("Error parsing response: {}", e.what());
    }

    return json;
}

void DedaoClient::initDefaultHeaders()
{
    std::string userAgent = std::string("User-Agent: ") + network::chrome;
    m_commonHeaders.add(userAgent);
    m_commonHeaders.add(network::accept_language);
    m_commonHeaders.add(network::accept_encoding);
    m_commonHeaders.add(std::string("HOST: www.dedao.cn"));
}

void DedaoClient::initDefaultOptions()
{
    constexpr time_t timeoutSecond = 5000;
    auto timeout = std::make_shared<network::TimeOut>(timeoutSecond);
    m_commonOptions.insert({timeout->getOption(), timeout});
    auto acceptEncoding = std::make_shared<network::AcceptEncoding>("gzip");
    m_commonOptions.insert({acceptEncoding->getOption(), acceptEncoding});
    auto sslVerifyHost = std::make_shared<network::SSLVerifyHost>(false);
    m_commonOptions.insert({sslVerifyHost->getOption(), sslVerifyHost});
    auto sslVerifyPeer = std::make_shared<network::SSLVerifyPeer>(false);
    m_commonOptions.insert({sslVerifyPeer->getOption(), sslVerifyPeer});
}

network::CurlHeader DedaoClient::createLiveHeader(const std::string& alias_id)
{
    std::string referer = "Referer: https://www.dedao.cn/live/detail?id=" + alias_id;
    network::CurlHeader headers;
    headers.add("Content-Type: application/json");
    headers.add(referer);
    headers.add(std::string("Origin: " + dedaoHomeUrl));
    headers.add("xi-csrf-token:" + csrfToken());
    headers.add("xi-dt: web");

    return headers;
}

network::CurlHeader DedaoClient::createCourseHeader(const std::string& detail_id)
{
    std::string referer = "Referer: https://www.dedao.cn/course/detail?id=" + detail_id;
    network::CurlHeader headers;
    headers.add("Content-Type: application/json");
    headers.add(referer);
    headers.add(std::string("Origin: " + dedaoHomeUrl));
    headers.add("xi-csrf-token:" + csrfToken());
    headers.add("xi-dt: web");

    return headers;
}

}  // namespace dedaoapi
