#include <regex>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>

#include "WeiboClient.h"
#include "WeiboApiConstants.h"
#include "Util/JsonProcess.h"
#include "WeiboLog.h"
#include "WeiboUtils.h"
#include "NetWork/NetworkLog.h"
#include "NetWork/HeaderBodyResponseWrapper.h"
#include "NetWork/CurlCpp/CurlCookie.h"
#include "NetWork/CurlCpp/CurlCookieOpt.h"

namespace weiboapi
{
static constexpr int seconds = 60;
static constexpr int minutes = 60;
static constexpr int hours = 24;
static constexpr int daySeconds = hours * minutes * seconds;  // 24*60*60

std::string_view extractJsonFromJsonp(std::string_view jsonp)
{
    size_t start = jsonp.find('(');
    size_t end = jsonp.rfind(')');
    if (start == std::string_view::npos || end == std::string_view::npos || end <= start)
    {
        throw std::runtime_error("Invalid JSONP format");
    }
    return jsonp.substr(start + 1, end - start - 1);
}

WeiboClient::WeiboClient()
{
    initDefaultHeaders();
    initDefaultOptions();
}

WeiboClient& WeiboClient::globalClient()
{
    static WeiboClient WeiboClient;
    return WeiboClient;
}

ComponentPlayPlayinfoResponse WeiboClient::getPlayInfoByMid(const std::string& mid)
{
    namespace chr = std::chrono;
    std::string page = "%2Ftv%2Fshow%2F1034%3A" + mid;
    std::string url = weiboapi::weiboVideoInfoUrl + page;
    nlohmann::json content = nlohmann::json::parse(weiboapi::weiboPostContent);
    content["Component_Play_Playinfo"]["oid"] = "1034:" + mid;
    std::string param = "data=" + content.dump();
    std::string response;
    network::CurlHeader header;
    std::string userAgent = std::string("User-Agent: ") + network::chrome;
    std::string acceptEncoding = "Accept-Encoding: gzip, deflate";
    std::string accept = "Accept: application/json, text/plain, */*";
    std::string content_type = "Content-Type: application/x-www-form-urlencoded";
    std::string origin = std::string("origin: ") + weiboapi::weiboHomeUrl;
    std::string x_xsrf_token = std::string("x-xsrf-token: ") + m_cookies.cookie(".weibo.com").value("XSRF-TOKEN");
    std::string referer = std::string("referer: ") + weiboapi::weiboMidRefererUrl + "1034:" + mid + "?from=old_pc_videoshow";
    header.add(userAgent);
    header.add(acceptEncoding);
    header.add(accept);
    header.add(content_type);
    header.add(origin);
    header.add(x_xsrf_token);
    header.add(referer);

    post(url, response, param, header, false, CurlOptions(), true);

    std::ofstream test("test.json");
    test << response;
    test.close();

    ComponentPlayPlayinfoResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what();
    }

    return ret;
}

WeiboAjaxData WeiboClient::getPlayInfoByWid(const std::string& wid)
{
    std::string url = weiboapi::weiboAjaxUrl + wid;
    std::string response;
    network::CurlHeader header;
    std::string userAgent = std::string("User-Agent: ") + network::chrome;
    std::string accept_encoding = "Accept-Encoding: gzip, deflate, br, zstd";
    std::string accept = "Accept: application/json, text/plain, */*";

    get(url, response, header, false, CurlOptions(), true);

    WeiboAjaxData ret;
    ret = getDataFromRespones(response);

    return ret;
}

UserMblogResponse WeiboClient::getUserMlog(const std::string& uid)
{
    std::string url = weiboapi::weiboUserMblogUrl + uid;
    std::string response;
    network::CurlHeader header;
    std::string userAgent = std::string("User-Agent: ") + network::chrome;
    std::string accept_encoding = "Accept-Encoding: gzip, deflate, br, zstd";
    std::string accept = "Accept: application/json, text/plain, */*";

    get(url, response, header, false, CurlOptions(), true);

    UserMblogResponse ret;
    ret = getDataFromRespones(response);

    return ret;
}

std::string WeiboClient::getStreamInfo(const std::string& wid)
{
    std::string mid;
    if (wid.find(":") != std::string::npos)
    {
        mid = wid.substr(wid.find(":") + 1);
    }
    auto ret = getPlayInfoByMid(mid);
    return ret.data.Component_Play_Playinfo.stream_url;
}

QRCResponse WeiboClient::getLoginUrl()
{
    namespace chr = std::chrono;
    std::string response;
    std::string timestamp = std::to_string(chr::duration_cast<chr::milliseconds>(chr::system_clock::now().time_since_epoch()).count());
    const ParamType params{
        {"entry",    "weibo"  },
        {"size",     "180"    },
        {"callback", timestamp},
    };
    network::CurlHeader header;
    std::string userAgent = std::string("User-Agent: ") + network::chrome;
    std::string acceptEncoding = "Accept-Encoding: gzip, deflate";
    std::string accept = "Accept: */*";
    std::string keep_alive = "Connection: keep-alive";
    std::string referer = std::string("Referer: ") + weiboapi::weiboReferer;
    header.add(userAgent);
    header.add(acceptEncoding);
    header.add(accept);
    header.add(keep_alive);
    header.add(referer);

    get(weiboapi::weiboQrcodeUrl, response, params, header, false);

    QRCResponse ret;
    ret = getDataFromPJsonRespones(response);
    return ret;
}

bool WeiboClient::getQrcImage(const std::string& url, const std::filesystem::path& path)
{
    FILE* file = fopen(path.string().c_str(), "wb");
    if (!file)
    {
        std::string str = strerror(errno);
        WEIBO_LOG_ERROR("fopen error: {}, filePath: {}", str, path.string());
        return false;
    }

    network::CurlHeader header;
    header.add(std::string("referer: ") + weiboapi::weiboReferer);
    get(url, file, header, true);
    fclose(file);

    return true;
}

LoginScaningStatus WeiboClient::getLoginStatus(const std::string& qrid)
{
    namespace chr = std::chrono;
    std::string response;
    std::string timestamp = std::to_string(chr::duration_cast<chr::milliseconds>(chr::system_clock::now().time_since_epoch()).count());
    const ParamType params{
        {"entry",    "weibo"           },
        {"qrid",     qrid              },
        {"callback", "STK_" + timestamp},
    };

    network::CurlHeader header;
    header.add(std::string("referer: ") + weiboapi::weiboReferer);
    get(weiboapi::weiboCheckUrl, response, params, header, true);

    LoginScaningStatus ret;
    ret = getDataFromPJsonRespones(response);
    return ret;
}

LoginInfo WeiboClient::loginWeibo(const std::string alt)
{
    namespace chr = std::chrono;
    std::string response;
    std::string timestamp = std::to_string(chr::duration_cast<chr::milliseconds>(chr::system_clock::now().time_since_epoch()).count());
    const ParamType params{
        {"entry",       "weibo"           },
        {"returntype",  "TEXT"            },
        {"crossdomain", "1"               },
        {"cdult",       "3"               },
        {"domain",      "weibo.com"       },
        {"alt",         alt               },
        {"savestate",   "30"              },
        {"callback",    "STK_" + timestamp},
    };

    CurlOptions options;
    auto followLocation = std::make_shared<network::FollowLocation>(1);
    options.insert({followLocation->getOption(), followLocation});
    get(weiboapi::weiboLoginUrl, response, params, network::CurlHeader(), true, options, true);

    LoginInfo ret;
    ret = getDataFromPJsonRespones(response);

    return ret;
}

bool WeiboClient::crossDomainRequest(std::vector<std::string> urls)
{
    urls.push_back(weiboapi::weiboHomeUrl);
    for (const auto& url : urls)
    {
        network::ResponseHeaderAndBody response;

        get(url, response);

        auto header = parseHeader(response.header);
        if (header.end() != header.find(network::set_cookies))
        {
            // WEIBO_LOG_INFO("Login success!");
            std::lock_guard lk(m_mutexRequest);
            std::string cookie = header.at(network::set_cookies);
            if (url == weiboapi::weiboHomeUrl)
            {
                cookie += "; ";
                cookie += network::domain_key;
                cookie += "=.weibo.com";
            }
            m_cookies.addCurlCookies(cookie);
            m_commonOptions[network::CookieFileds::opt] = std::make_shared<network::CookieFileds>(m_cookies.cookie(".weibo.com"));
        }
    }

    return true;
}

bool WeiboClient::isLogined() const
{
    return m_cookies.cookie(".weibo.com").contains("XSRF-TOKEN");
}

std::string WeiboClient::cookie() const
{
    return network::CurlCookieOpt(m_cookies.cookie(".weibo.com")).shortContent();
}

void WeiboClient::getDetailInfo(const std::string& mid)
{
}

void WeiboClient::parseCookie(const std::string& url)
{
}

nlohmann::json WeiboClient::getDataFromPJsonRespones(const std::string& respones)
{
    nlohmann::json json;
    try
    {
        auto realJson = extractJsonFromJsonp(respones);
        json = nlohmann::json::parse(realJson);
        util::JsonProcess::removeNullValues(json);
    }
    catch (std::exception& e)
    {
        WEIBO_LOG_ERROR("Error parsing response: {}", e.what());
    }

    return json;
}

nlohmann::json WeiboClient::getDataFromRespones(const std::string& respones)
{
    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(respones);
        util::JsonProcess::removeNullValues(json);
    }
    catch (std::exception& e)
    {
        WEIBO_LOG_ERROR("Error parsing response: {}", e.what());
    }

    return json;
}

network::CurlHeader WeiboClient::passPortHeaders()
{
    network::CurlHeader header;

    std::string userAgent = std::string("user-agent: ") + network::chrome;
    header.add(userAgent);

    return header;
}

void WeiboClient::initDefaultHeaders()
{
    std::string userAgent = std::string("User-Agent: ") + network::chrome;
    m_commonHeaders.add(userAgent);
    m_commonHeaders.add(network::accept_language);
    m_commonHeaders.add(network::accept_encoding);
}

void WeiboClient::initDefaultOptions()
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

}  // namespace weiboapi
