#include <regex>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>
#include <mutex>

#include "XHSClient.h"
#include "XHSApiConstants.h"
#include "Util/JsonProcess.h"
#include "PluginCrypto/Encoding.h"
#include "NetWork/HeaderBodyResponseWrapper.h"
#include "NetWork/CurlCpp/CurlCookie.h"
#include "NetWork/CurlCpp/CurlOption.h"
#include "XHSLog.h"
#include "XHSUtils.h"

namespace xhsapi
{

XHSClient::XHSClient()
{
    initDefaultOptions();
    registerAnonymous();
}

XHSClient& XHSClient::globalClient()
{
    static XHSClient client;
    return client;
}

QRCodeResponse XHSClient::getQRCode()
{
    std::string url = apiBase + Login::QRCreate;
    nlohmann::json data;
    data["qr_type"] = 1;
    std::string dataStr = data.dump();
    network::CurlHeader headers = createSignedHeaders("POST", Login::QRCreate, dataStr);

    std::string response;
    post(url, response, dataStr, headers, false);

    QRCodeResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("getQRCode response parsing failed: {}", e.what());
    }

    return ret;
}

LoginStatusResponse XHSClient::getLoginStatus(const std::string& code, const std::string& qrId)
{
    std::string url = apiBase + Login::QRCheck;
    nlohmann::json data;
    data["code"] = code;
    data["qrId"] = qrId;
    std::string dataStr = data.dump();
    network::CurlHeader headers = createSignedHeaders("POST", Login::QRCheck, dataStr);
    headers.add("service-tag: webcn");

    std::string response;
    post(url, response, dataStr, headers, false);

    LoginStatusResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("getLoginStatus response parsing failed: {}", e.what());
    }

    return ret;
}

AccountInfoResponse XHSClient::getAccountInfo()
{
    std::string url = apiBase + Login::AccountInfo;
    network::CurlHeader headers = createSignedHeaders("GET", Login::AccountInfo, "", SignFormat::Xyw);

    std::string response;
    get(url, response, headers, false);

    AccountInfoResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("getAccountInfo response parsing failed: {}", e.what());
    }

    return ret;
}

NoteDetailResponse XHSClient::getNoteDetail(const std::string& noteId, const std::string& xsecToken, const std::string& xsecSource)
{
    std::string url = apiBase + Api::NoteDetail;
    nlohmann::json data;
    data["extra"] = {
        {"need_body_topic", "1"}
    };
    data["image_formats"] = {"jpg", "webp", "avif"};
    data["source_note_id"] = noteId;
    data["xsec_token"] = xsecToken;
    data["xsec_source"] = xsecSource;
    std::string dataStr = data.dump();
    network::CurlHeader headers = createSignedHeaders("POST", Api::NoteDetail, dataStr);

    std::string response;
    post(url, response, dataStr, headers, false);

    NoteDetailResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("getNoteDetail response parsing failed, noteId: {}, error: {}", noteId, e.what());
    }

    return ret;
}

NoteItemListResponse XHSClient::getAccountNotes(const std::string& userId, const std::string& cursor, int num, const std::string& xsecToken,
                                                const std::string& xsecSource)
{
    std::string url = apiBase + Api::UserAllNotes;
    ParamType data;
    data["cursor"] = cursor;
    data["num"] = std::to_string(num);
    data["user_id"] = userId;
    data["image_formats"] = "jpg,webp,avif";
    data["xsec_token"] = xsecToken;
    data["xsec_source"] = xsecSource;

    const auto query = encodeData(data);
    network::CurlHeader headers = createSignedHeaders("GET", Api::UserAllNotes + "?" + query, "", SignFormat::Xyw);

    std::string response;
    get(url + "?" + query, response, headers, false);

    NoteItemListResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("getAccountNotes response parsing failed, userId: {}, error: {}", userId, e.what());
    }

    return ret;
}

NoteItemListResponse XHSClient::getFavoriteNotes(const std::string& userId, const std::string& cursor, int num, const std::string& xsecToken,
                                                 const std::string& xsecSource)
{
    std::string url = apiBase + Api::FavoriteNotes;
    ParamType data;
    data["cursor"] = cursor;
    data["num"] = std::to_string(num);
    data["user_id"] = userId;
    data["image_formats"] = "jpg,webp,avif";
    data["xsec_token"] = xsecToken;
    data["xsec_source"] = xsecSource;

    const auto query = encodeData(data);
    network::CurlHeader headers = createSignedHeaders("GET", Api::FavoriteNotes + "?" + query, "", SignFormat::Xyw);

    std::string response;
    get(url + "?" + query, response, headers, false);

    NoteItemListResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("getFavoriteNotes response parsing failed, userId: {}, error: {}", userId, e.what());
    }

    return ret;
}

bool XHSClient::isLogined() const
{
    std::lock_guard lk(m_mutexRequest);
    const auto cookie = m_cookies.cookie(domain).content();
    return cookie.find("web_session=") != std::string::npos;
}

std::string XHSClient::cookies() const
{
    std::lock_guard lk(m_mutexRequest);
    return std::string(m_cookies);
}

void XHSClient::setCookies(std::string cookies)
{
    std::lock_guard lk(m_mutexRequest);
    m_cookies = network::CurlCookies(cookies);
    m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
}

void XHSClient::initDefaultOptions()
{
    constexpr long timeoutSecond = 5;
    auto timeout = std::make_shared<network::TimeOut>(timeoutSecond);
    m_commonOptions.insert({timeout->getOption(), timeout});
    auto acceptEncoding = std::make_shared<network::AcceptEncoding>("gzip");
    m_commonOptions.insert({acceptEncoding->getOption(), acceptEncoding});
}

void XHSClient::registerAnonymous()
{
    std::string url = home;

    network::CurlHeader headers;
    headers.add("User-Agent: " + std::string(network::chrome));
    headers.add("Referer: " + std::string(home) + "/");
    headers.add("Origin: " + std::string(home));
    headers.add(
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7");
    headers.add("Accept-Language: en,zh-CN;q=0.9,zh;q=0.8");
    headers.add("Priority: u=1, i");
    headers.add("Accept-Encoding: gzip, deflate, br, zstd");

    network::ResponseHeaderAndBody response;
    get(url, response, headers, false);
    auto header = parseHeader(response.header);
    if (header.end() != header.find(network::set_cookies))
    {
        XHS_LOG_INFO("registerAnonymous success!");
        std::lock_guard lk(m_mutexRequest);
        m_cookies = network::CurlCookies().addCurlCookie(header.at(network::set_cookies));
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
    }

    setExploreCookie();

    std::pair<std::string, std::string> a1AndWebId = generateCookieA1AndWebId();
    network::CurlCookie cookie;
    cookie.setDomain(domain).setName("a1").setValue(a1AndWebId.first).setName("web_id").setValue(a1AndWebId.second);
    {
        std::lock_guard lk(m_mutexRequest);
        m_cookies.addCurlCookie(cookie);
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
    }

    return;
}

void XHSClient::setExploreCookie()
{
    std::string url = home + Api::Explore;

    network::CurlHeader headers;
    headers.add("User-Agent: " + std::string(network::chrome));
    headers.add(
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7");
    headers.add("Accept-Language: en,zh-CN;q=0.9,zh;q=0.8");
    headers.add("Priority: u=0, i");
    headers.add("Accept-Encoding: gzip, deflate, br, zstd");

    network::ResponseHeaderAndBody response;
    get(url, response, headers, false);
    auto header = parseHeader(response.header);
    if (header.end() != header.find(network::set_cookies))
    {
        XHS_LOG_INFO("setExploreHeaders success!");
        network::CurlCookie cookie(header.at(network::set_cookies));
        cookie.setDomain(domain);

        std::lock_guard lk(m_mutexRequest);
        m_cookies.addCurlCookie(cookie);
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
    }
}

void XHSClient::setScriptingCookie()
{
    std::string url = asBase + Api::Scripting;

    network::CurlHeader headers = createLoginHeaders();

    std::string data = scriptingData;
    std::string response;
    post(url, response, data, headers, false);

    ScriptingDataResponse ret;

    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("setScriptingCookie response parsing failed: {}", e.what());
    }

    if (!ret.data.data.empty())
    {
        XHS_LOG_INFO("setScriptingCookie success!");

        std::string websection = genWebsection(ret.data.data);
        network::CurlCookie cookie;
        cookie.setDomain(domain).setName("websectiga").setValue(websection);

        {
            std::lock_guard lk(m_mutexRequest);
            m_cookies.addCurlCookie(cookie);
            m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
        }
    }
}

void XHSClient::setActivateCookie()
{
    std::string url = apiBase + Login::Activate;
}

network::CurlHeader XHSClient::createLoginHeaders()
{
    network::CurlHeader headers;
    headers.add("User-Agent: " + std::string(network::chrome));
    headers.add("Referer: " + std::string(home) + "/");
    headers.add("Origin: " + std::string(home));
    headers.add("Accept: application/json, text/plain, */*");
    headers.add("Accept-Language: en,zh-CN;q=0.9,zh;q=0.8");
    headers.add("Priority: u=1, i");
    headers.add("Accept-Encoding: gzip, deflate, br, zstd");
    headers.add("Content-Type: application/json;charset=UTF-8");
    return headers;
}

network::CurlHeader XHSClient::createSignedHeaders(const std::string& method, const std::string& uri, const std::string& body, SignFormat format) const
{
    std::string cookieHeader;
    {
        std::lock_guard lk(m_mutexRequest);
        cookieHeader = m_cookies.cookie(domain).content();
    }
    const auto signature = signRequest(method, uri, body, parseSignCookies(cookieHeader), format);
    auto headers = createLoginHeaders();
    headers.add("x-s: " + signature.xS);
    headers.add("x-s-common: " + signature.xSCommon);
    headers.add("x-t: " + signature.xT);
    return headers;
}

std::string XHSClient::encodeData(const ParamType& params)
{
    std::stringstream ss;
    for (const auto& [key, value] : params)
    {
        ss << encoding::urlEncode(key) + "=" << encoding::urlEncode(value) << "&";
    }
    std::string paramsStr = ss.str();
    paramsStr.pop_back();

    return paramsStr;
}

nlohmann::json XHSClient::getDataFromRespones(const std::string& respones)
{
    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(respones);
        util::JsonProcess::removeNullValues(json);
    }
    catch (std::exception& e)
    {
        XHS_LOG_ERROR("Error parsing response: {}", e.what());
    }

    return json;
}

}  // namespace xhsapi
