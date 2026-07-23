#include <regex>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>
#include <mutex>

#include "DouYinClient.h"
#include "DouYinConstants.h"
#include "Util/JsonProcess.h"
#include "PluginCrypto/Encoding.h"
#include "NetWork/HeaderBodyResponseWrapper.h"
#include "NetWork/CurlCpp/CurlCookie.h"
#include "NetWork/CurlCpp/CurlOption.h"
#include "DouYinUtils.h"
#include "DouYinLog.h"

namespace douyinapi
{

const network::NetWork::ParamType loginParams{
    {"is_from_ttaccountsdk",   "1"                     },
    {"aid",                    "6383"                  },
    {"language",               "zh"                    },
    {"account_app_language",   "en"                    },
    {"next",                   "https://www.douyin.com"},
    {"is_new_login",           "1"                     },
    {"is_from_iesaccountsaas", "1"                     },
    {"device_platform",        "web_app"               },
};

const network::NetWork::ParamType checkData = {
    {"need_logo",      "false"                 },
    {"is_frontier",    "true"                  },
    {"is_new_login",   "1"                     },
    {"next",           "https://www.douyin.com"},
    {"need_short_url", "true"                  },
};

const network::NetWork::ParamType detailParams{
    {"device_platform",          "webapp"        },
    {"aid",                      "6383"          },
    {"channel",                  "channel_pc_web"},
    {"request_source",           "600"           },
    {"origin_type",              "quick_player"  },
    {"update_version_code",      "170400"        },
    {"pc_client_type",           "1"             },
    {"pc_libra_divert",          "Windows"       },
    {"support_h265",             "1"             },
    {"support_dash",             "1"             },
    {"webcast_sdk_version",      "170400"        },
    {"webcast_sdk_version_code", "170400"        },
    {"version_code",             "170400"        },
    {"version_name",             "17.4.0"        },
    {"cookie_enabled",           "true"          },
    {"screen_width",             "1536"          },
    {"screen_height",            "864"           },
    {"browser_language",         "en"            },
    {"browser_platform",         "Win32"         },
    {"browser_name",             "Chrome"        },
    {"browser_version",          "146.0.0.0"     },
    {"browser_online",           "true"          },
    {"engine_name",              "Blink"         },
    {"engine_version",           "146.0.0.0"     },
    {"os_name",                  "Windows"       },
    {"os_version",               "10"            },
    {"cpu_core_num",             "16"            },
    {"device_memory",            "8"             },
    {"platform",                 "PC"            },
    {"downlink",                 "10"            },
    {"effective_type",           "4g"            },
    {"round_trip_time",          "200"           },
    {"msToken",                  ""              },
    {"webid",                    ""              },
};

constexpr const char kUserAgent[] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/146.0.0.0 Safari/537.36";

DouYinClient::DouYinClient()
{
    registerAnonymous();
    initDefaultOptions();
}

DouYinClient& DouYinClient::globalClient()
{
    static DouYinClient client;
    return client;
}

QRCodeResponse DouYinClient::getQRCode()
{
    std::string url = login + Login::QRCreate;
    network::CurlHeader headers = createLoginHeaders();
    ParamType params = loginParams;
    std::string paramsStr = encryptionParams(params);
    url += "?" + paramsStr;

    std::string response;
    get(url, response, headers);

    QRCodeResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }
    return ret;
}

QRCodeStatusResponse DouYinClient::getLoginStatus(const std::string& token, const std::string& captcha)
{
    std::string url = login + Login::QRCheck;
    network::CurlHeader headers = createLoginHeaders();
    headers.add("Content-type: application/x-www-form-urlencoded");

    ParamType check = checkData;
    check["token"] = token;
    if (!captcha.empty())
    {
        check["captcha"] = captcha;
    }
    const std::string data = encodeData(check);
    ParamType params = loginParams;
    const std::string paramsStr = encryptionParams(params, data);
    url += "?" + paramsStr;

    network::ResponseHeaderAndBody response;
    post(url, response, data, headers, false);
    auto header = parseHeader(response.header);
    if (header.end() != header.find(network::set_cookies))
    {
        DOUYIN_LOG_INFO("Login success!");
        std::lock_guard lk(m_mutexRequest);
        network::CurlCookie cookie(header.at(network::set_cookies));
        cookie.setDomain(domain);
        m_cookies.addCurlCookie(cookie);
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
    }

    QRCodeStatusResponse ret;
    try
    {
        ret = getDataFromRespones(response.body);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response.body);
    }

    return ret;
}

AccountInfoResponse DouYinClient::getAccountInfo()
{
    std::string url = login + Login::AccountInfo;
    network::CurlHeader headers = createLoginHeaders();
    ParamType params = loginParams;
    std::string paramsStr = encryptionParams(params);
    url += "?" + paramsStr;

    std::string response;
    get(url, response, headers, false);

    AccountInfoResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }
    return ret;
}

FollowingResponse DouYinClient::getFollowing()
{
    std::string url = home + Api::Relation;
    network::CurlHeader headers = createDetailHeaders();
    ParamType params = loginParams;
    params["count"] = "100";
    params["source"] = "coldup";
    params["need_remove_share_panel"] = "true";
    params["need_sorted_info"] = "true";
    std::string paramsStr = encryptionParams(params);
    url += "?" + paramsStr;

    std::string response;
    get(url, response, headers, false);

    FollowingResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }
    return ret;
}

AwemeDetailResponse DouYinClient::getAwemeDetail(const std::string& awemeId)
{
    std::string url = home + Api::Detail;
    network::CurlHeader headers = createDetailHeaders();

    std::string msToken;
    std::string webId;
    std::string uifid;
    {
        std::lock_guard lk(m_mutexRequest);
        msToken = m_msToken;
        webId = m_webId;
        uifid = m_cookies.cookie(domain).value("UIFID");
    }

    ParamType params = detailParams;
    params["aweme_id"] = awemeId;
    params["web_id"] = webId;
    if (!msToken.empty())
    {
        params["msToken"] = msToken;
    }
    if (!uifid.empty())
    {
        params["uifid"] = uifid;
    }

    std::string paramsStr = encryptionParams(params);
    url += "?" + paramsStr;

    std::string response;
    get(url, response, headers);

    AwemeDetailResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

SeriesDetail DouYinClient::getSeriesDetail(const std::string& seriesId, int cursor, int count)
{
    std::string url = home + Api::Series;
    network::CurlHeader headers = createDetailHeaders();

    ParamType params = detailParams;
    params["series_id"] = seriesId;
    params["cursor"] = std::to_string(cursor);
    params["count"] = std::to_string(count);
    params["pull_type"] = "2";
    std::string paramsStr = encryptionParams(params);
    url += "?" + paramsStr;

    std::string response;
    get(url, response, headers);

    SeriesDetail ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

SeriesDetail DouYinClient::getMixDetail(const std::string& mixId, int cursor, int count)
{
    std::string url = home + Api::Mix;
    network::CurlHeader headers = createDetailHeaders();

    ParamType params = detailParams;
    params["mix_id"] = mixId;
    params["cursor"] = std::to_string(cursor);
    params["count"] = std::to_string(count);

    std::string paramsStr = encryptionParams(params);
    url += "?" + paramsStr;

    std::string response;
    get(url, response, headers);

    SeriesDetail ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

SeriesDetail DouYinClient::getUserAll(const std::string& userId, int cursor, int count)
{
    std::string url = home + Api::UserAll;
    network::CurlHeader headers = createDetailHeaders();

    ParamType params = detailParams;
    params["sec_user_id"] = userId;
    params["max_cursor"] = std::to_string(cursor);
    params["count"] = std::to_string(count);

    std::string paramsStr = encryptionParams(params);
    url += "?" + paramsStr;

    std::string response;
    get(url, response, headers);

    SeriesDetail ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

SeriesDetail DouYinClient::getUserHistory(int cursor, int count)
{
    std::string url = home + Api::UserHistory;
    network::CurlHeader headers = createDetailHeaders();

    ParamType params = detailParams;
    params["max_cursor"] = std::to_string(cursor);
    params["count"] = std::to_string(count);

    std::string paramsStr = encryptionParams(params);
    url += "?" + paramsStr;

    std::string response;
    get(url, response, headers);

    SeriesDetail ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

SeriesDetail DouYinClient::getUserCollection(int cursor, int count)
{
    std::string url = home + Api::ListCollection;
    network::CurlHeader headers = createDetailHeaders();

    ParamType dataParam;
    dataParam["cursor"] = std::to_string(cursor);
    dataParam["count"] = std::to_string(count);

    const std::string data = encodeData(dataParam);
    ParamType params = detailParams;
    const std::string paramsStr = encryptionParams(params, data);
    url += "?" + paramsStr;

    std::string response;
    post(url, response, data, headers, true);

    SeriesDetail ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    return ret;
}

bool DouYinClient::isLogined() const
{
    std::lock_guard lk(m_mutexRequest);
    const auto cookie = m_cookies.cookie(domain);
    return !cookie.value("sessionid").empty() || !cookie.value("sessionid_ss").empty() || !cookie.value("sid_guard").empty();
}

std::string DouYinClient::cookies() const
{
    std::lock_guard lk(m_mutexRequest);
    return std::string(m_cookies);
}

void DouYinClient::setCookies(std::string cookies)
{
    std::lock_guard lk(m_mutexRequest);
    m_cookies = network::CurlCookies(cookies);
    const auto cookie = m_cookies.cookie(domain);
    m_msToken = cookie.value("msToken");
    m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
}

void DouYinClient::clearSession()
{
    std::lock_guard lk(m_mutexRequest);
    m_cookies = {};
    m_userId.clear();
    m_msToken.clear();
    m_commonOptions.erase(network::CookieFields::opt);
}

std::string DouYinClient::encodeData(const ParamType& params)
{
    std::stringstream ss;
    for (const auto& [key, value] : params)
    {
        ss << encoding::urlEncode(key) + "=" << encoding::urlEncode(value) << "&";
    }
    std::string paramsStr = ss.str();
    if (!paramsStr.empty())
    {
        paramsStr.pop_back();
    }

    return paramsStr;
}

std::string DouYinClient::encryptionParams(const ParamType& params, const std::string& body)
{
    std::string paramsStr = encodeData(params);
    ABogus bogus(kUserAgent);
    std::string abogus = bogus.getValue(paramsStr, body);

    paramsStr += "&a_bogus=" + abogus;

    return paramsStr;
}

bool DouYinClient::downloadImage(const std::string& url, const std::filesystem::path& path)
{
    FILE* file = fopen(path.string().c_str(), "wb");
    if (!file)
    {
        std::string str = strerror(errno);
        DOUYIN_LOG_ERROR("fopen error: {}, filePath: {}", str, path.string());
        return false;
    }

    network::CurlHeader header;
    header.add(std::string("Referer: ") + douyinapi::home);
    get(url, file, header, true);
    fclose(file);

    return true;
}

void DouYinClient::initDefaultOptions()
{
    constexpr long timeoutSecond = 5;
    auto timeout = std::make_shared<network::TimeOut>(timeoutSecond);
    m_commonOptions.insert({timeout->getOption(), timeout});
    auto acceptEncoding = std::make_shared<network::AcceptEncoding>("gzip");
    m_commonOptions.insert({acceptEncoding->getOption(), acceptEncoding});
}

void DouYinClient::registerAnonymous()
{
    requestHome();
    registerTtWid();
    getMsToken();
    getWebId();
}

void DouYinClient::requestHome()
{
    std::string url = home;

    network::CurlHeader headers;
    headers.add("User-Agent: " + std::string(kUserAgent));
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
        DOUYIN_LOG_INFO("registerAnonymous success!");
        std::lock_guard lk(m_mutexRequest);
        network::CurlCookie cookie = network::CurlCookie(header.at(network::set_cookies));
        cookie.setDomain(domain);
        m_cookies = network::CurlCookies().addCurlCookie(cookie);
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
    }
}

void DouYinClient::registerTtWid()
{
    std::string url = douyinapi::TtWidUrl;

    network::CurlHeader headers = createParamHeaders();

    network::ResponseHeaderAndBody response;
    post(url, response, kTtWidData, headers, false);

    auto header = parseHeader(response.header);
    if (header.end() != header.find(network::set_cookies))
    {
        DOUYIN_LOG_INFO("registerTtWid success!");
        std::lock_guard lk(m_mutexRequest);
        network::CurlCookie cookie = network::CurlCookie(header.at(network::set_cookies));
        cookie.setDomain(domain);
        m_cookies.addCurlCookie(cookie);
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
    }
}

void DouYinClient::getMsToken()
{
    std::string url = douyinapi::MsTokenUrl;

    network::CurlHeader headers = createParamHeaders();
    auto data = nlohmann::json::parse(kMsTokenData);
    auto time = std::chrono::system_clock::now().time_since_epoch();
    data["tspFromClient"] = std::chrono::duration_cast<std::chrono::milliseconds>(time).count();

    url += std::string("?token=") + kMsToken;

    network::ResponseHeaderAndBody response;
    post(url, response, data.dump(), headers, false);

    auto header = parseHeader(response.header);
    if (header.end() != header.find(network::set_cookies))
    {
        DOUYIN_LOG_INFO("getMsToken success!");
        std::lock_guard lk(m_mutexRequest);
        network::CurlCookie cookie = network::CurlCookie(header.at(network::set_cookies));
        cookie.setDomain(domain);
        m_msToken = cookie.value("msToken");
        m_cookies.addCurlCookie(cookie);
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
    }
}

void DouYinClient::getWebId()
{
    std::string url = douyinapi::WebIdUrl + std::string("?") + webIdParam;
    ;
    network::CurlHeader headers = createParamHeaders();

    std::string response;
    post(url, response, webIdData, headers, false);

    WebIdResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        DOUYIN_LOG_ERROR("parsing response error, error msg : {}, response : {}", e.what(), response);
    }

    if (!ret.web_id.empty())
    {
        DOUYIN_LOG_INFO("getWebId success!");
        {
            std::lock_guard lk(m_mutexRequest);
            m_webId = ret.web_id;
        }
    }
}

network::CurlHeader DouYinClient::createLoginHeaders()
{
    network::CurlHeader headers;
    headers.add("User-Agent: " + std::string(kUserAgent));
    headers.add("Referer: " + std::string(home) + "/");
    headers.add("Origin: " + std::string(home));
    headers.add("Accept: application/json");
    headers.add("Accept-Language: en,zh-CN;q=0.9,zh;q=0.8");
    headers.add("Priority: u=1, i");
    headers.add("Accept-Encoding: gzip, deflate, br, zstd");
    return headers;
}

network::CurlHeader DouYinClient::createDetailHeaders()
{
    network::CurlHeader headers;
    headers.add("User-Agent: " + std::string(kUserAgent));
    headers.add("Referer: " + std::string(home) + "/?recommend=1");
    headers.add("Accept: */*");
    headers.add("Accept-Encoding: */*");
    return headers;
}

network::CurlHeader DouYinClient::createParamHeaders()
{
    network::CurlHeader headers;
    headers.add("User-Agent: " + std::string(kUserAgent));
    headers.add("Content-Type: text/plain;charset=UTF-8");
    headers.add("Referer: " + std::string(home) + "/?recommend=1");
    headers.add("Accept: */*");
    headers.add("Accept-Encoding: */*");
    return headers;
}

network::CurlHeader DouYinClient::createCollectionHeaders()
{
    std::string userId;
    {
        std::lock_guard lk(m_mutexRequest);
        userId = m_userId;
    }

    network::CurlHeader headers;
    headers.add("User-Agent: " + std::string(kUserAgent));
    headers.add("Referer: " + std::string(home) + "/" + userId + "?from_tab_name=main&showSubTab=video");
    headers.add("Origin: " + std::string(home));
    headers.add("Accept: application/json");
    headers.add("Accept-Language: en,zh-CN;q=0.9,zh;q=0.8");
    headers.add("Priority: u=1, i");
    headers.add("Accept-Encoding: gzip, deflate, br, zstd");
    return headers;
}

nlohmann::json DouYinClient::getDataFromRespones(const std::string& respones)
{
    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(respones);
        util::JsonProcess::removeNullValues(json);
    }
    catch (std::exception& e)
    {
        DOUYIN_LOG_ERROR("Error parsing response: {}", e.what());
    }

    return json;
}

}  // namespace douyinapi
