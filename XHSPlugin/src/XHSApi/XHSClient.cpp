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
    network::CurlHeader headers = createLoginHeaders();
    nlohmann::json data;
    data["qr_type"] = 1;
    std::string dataStr = data.dump();

    std::string response;
    post(url, response, dataStr, headers, false);

    QRCodeResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

LoginStatusResponse XHSClient::getLoginStatus(const std::string& code, const std::string& qrId)
{
    std::string url = apiBase + Login::QRCheck;
    network::CurlHeader headers = createLoginHeaders();
    nlohmann::json data;
    data["code"] = code;
    data["qr_id"] = qrId;
    std::string dataStr = data.dump();

    std::string response;
    post(url, response, dataStr, headers, false);

    LoginStatusResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

AccountInfoResponse XHSClient::getAccountInfo()
{
    std::string url = apiBase + Login::AccountInfo;
    network::CurlHeader headers = createLoginHeaders();

    std::string response;
    get(url, response, headers, false);

    AccountInfoResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

NoteDetailResponse XHSClient::getNoteDetail(const std::string& noteId, const std::string& xsecToken, const std::string& xsecSource)
{
    std::string url = apiBase + Login::QRCheck;
    network::CurlHeader headers = createLoginHeaders();
    nlohmann::json data = nlohmann::json::parse(feedData);
    data["source_note_id"] = noteId;
    data["xsec_token"] = xsecToken;
    data["xsec_source"] = xsecSource;
    std::string dataStr = data.dump();

    std::string response;
    post(url, response, dataStr, headers, false);

    NoteDetailResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

NoteItemListResponse XHSClient::getAccountNotes(const std::string& userId, int cursor, int num, const std::string& xsecToken, const std::string& xsecSource)
{
    std::string url = apiBase + Api::UserAllNotes;
    network::CurlHeader headers = createLoginHeaders();
    ParamType data;
    data["cursor"] = cursor;
    data["num"] = num;
    data["user_id"] = userId;
    data["image_formats"] = "jpg,webp,avif";
    data["xsec_token"] = xsecToken;
    data["xsec_source"] = xsecSource;

    std::string response;
    get(url, response, data, headers, false);

    NoteItemListResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

NoteItemListResponse XHSClient::getFavoriteNotes(const std::string& userId, int cursor, int num, const std::string& xsecToken, const std::string& xsecSource)
{
    std::string url = apiBase + Api::FavoriteNotes;
    network::CurlHeader headers = createLoginHeaders();
    ParamType data;
    data["cursor"] = cursor;
    data["num"] = num;
    data["user_id"] = userId;
    data["image_formats"] = "jpg,webp,avif";
    data["xsec_token"] = xsecToken;
    data["xsec_source"] = xsecSource;

    std::string response;
    get(url, response, data, headers, false);

    NoteItemListResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        XHS_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

bool XHSClient::isLogined() const
{
    return false;
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
        XHS_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
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
