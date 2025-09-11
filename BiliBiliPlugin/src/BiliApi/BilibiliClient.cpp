#include <regex>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>

#include "BilibiliClient.h"
#include "BiliApiConstants.h"
#include "Util/JsonProcess.h"
#include "Util/LocaleHelper.h"
#include "BilibiliLog.h"
#include "BilibiliUtils.h"
#include "NetWork/NetworkLog.h"
#include "NetWork/HeaderBodyResponseWrapper.h"
#include "NetWork/CurlCpp/CurlCookie.h"

namespace biliapi
{
static constexpr int seconds = 60;
static constexpr int minutes = 60;
static constexpr int hours = 24;
static constexpr int daySeconds = hours * minutes * seconds;  // 24*60*60

constexpr char cookie_dedeUserId[] = "DedeUserID";
constexpr char cookie_biliJct[] = "bili_jct";
constexpr char cookie_sessdata[] = "SESSDATA";
constexpr char domain[] = "bilibili.com";

std::string to_string(BusinessType businessType)
{
    switch (businessType)
    {
    case biliapi::BusinessType::Archive:
        return "archive";
    case biliapi::BusinessType::Pgc:
        return "pgc";
    case biliapi::BusinessType::Live:
        return "live";
    case biliapi::BusinessType::Article_list:
        return "article-list";
    case biliapi::BusinessType::Article:
        return "article";
    case biliapi::BusinessType::Unkown:
    default:
        break;
    }

    return std::string();
}

std::string to_string(HistoryType historyType)
{
    switch (historyType)
    {
    case biliapi::All:
        return "all";
    case biliapi::Archive:
        return "archive";
    case biliapi::Live:
        return "live";
    case biliapi::Article:
        return "article";
    case biliapi::Unkown:
    default:
        break;
    }
    return std::string();
}

network::NetWork::ParamType HistoryQueryParam::toParam() const
{
    network::NetWork::ParamType res;
    if (max != 0)
    {
        res.insert({"max", std::to_string(max)});
    }

    if (business != BusinessType::Unkown)
    {
        res.insert({"business", to_string(business)});
    }

    if (view_at != 0)
    {
        res.insert({"view_at", std::to_string(view_at)});
    }

    if ((type != HistoryType::Unkown && type != HistoryType::All))
    {
        res.insert({"type", to_string(type)});
    }

    if (ps != 0 && ps != 20)
    {
        res.insert({"ps", std::to_string(ps)});
    }

    return res;
}

std::string HistoryQueryParam::toString() const
{
    std::string res;
    auto param = toParam();
    for (const auto& [key, val] : param)
    {
        res += (key + ": " + val + ",");
    }
    return res;
}

BilibiliClient::BilibiliClient()
{
    initDefaultHeaders();
    initDefaultOptions();
}

BilibiliClient& BilibiliClient::globalClient()
{
    static BilibiliClient bilibiliClient;
    return bilibiliClient;
}

VideoViewOrigin BilibiliClient::getVideoView(const std::string& bvid)
{
    ParamType param;
    param.emplace("bvid", bvid);

    std::string response;
    get(VideoURL::View, response, param);
    VideoViewOrigin ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getVideoView error! bvid: {}, error: {}", bvid, e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

PlayUrlOrigin BilibiliClient::getPlayUrl(long long cid, long long qn, const std::string& bvid, long long fnval)
{
    ParamType param;
    param.emplace("bvid", bvid);
    param.emplace("cid", std::to_string(cid));
    param.emplace("qn", std::to_string(qn));
    param.emplace("fnver", "0");
    param.emplace("fnval", std::to_string(fnval));
    param.emplace("fourk", "1");

    encodeWithWbi(param);

    std::string response;
    get(VideoURL::Playurl, response, param);

    PlayUrlOrigin ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::string str = e.what();
        BILIBILI_LOG_ERROR("getPlayUrl error! cid: {},  qn: {},  bvid: {}, fnva: {}, error: {}", cid, qn, bvid, fnval, str);
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

LoginUrlOrigin BilibiliClient::getLoginUrl()
{
    std::string response;
    get(PassportURL::QRCode, response, passPortHeaders());

    LoginUrlOrigin ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getLoginUrl error! error: {}", e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

LoginStatusScanning BilibiliClient::getLoginStatus(const std::string& qrcodeKey)
{
    ParamType param;
    param["qrcode_key"] = qrcodeKey;

    network::ResponseHeaderAndBody response;
    get(PassportURL::LoginStatus, response, param, passPortHeaders());
    auto header = parseHeader(response.header);
    if (header.end() != header.find(network::set_cookies))
    {
        BILIBILI_LOG_INFO("Login success!");
        std::lock_guard lk(m_mutexRequest);
        m_cookies.addCurlCookies(header.at(network::set_cookies));
        m_commonOptions[network::CookieFileds::opt] = std::make_shared<network::CookieFileds>(m_cookies.cookie(domain));
    }

    LoginStatusScanning ret;
    try
    {
        ret = getDataFromRespones(response.body);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getLoginStatus error! qrcodeKey: {}, error: {}", qrcodeKey, e.what());
        BILIBILI_LOG_ERROR("response is {}", response.body);
    }

    return ret;
}

Nav BilibiliClient::getNavInfo()
{
    std::string response;
    get(PassportURL::WebNav, response, network::CurlHeader(), false, CurlOptions(), isLogined());

    Nav nav;
    try
    {
        nav = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getNavInfo error! error: {}", e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }

    return nav;
}

LogoutExitV2 BilibiliClient::getLogoutExitV2()
{
    std::string response;

    auto keys = m_cookies.cookie(domain).keys();
    int haveId = 3;

    for (const auto& key : keys)
    {
        if (key == cookie_dedeUserId || key == cookie_biliJct || key == cookie_sessdata)
        {
            --haveId;
        }
    }

    if (haveId == 0)
    {
        std::string cookie = "Cookie: ";
        cookie += std::string(cookie_dedeUserId) + "=" + m_cookies.cookie(domain).value(cookie_dedeUserId);
        cookie += "; ";
        cookie += std::string(cookie_biliJct) + "=" + m_cookies.cookie(domain).value(cookie_biliJct);
        cookie += "; ";
        cookie += std::string(cookie_sessdata) + "=" + m_cookies.cookie(domain).value(cookie_sessdata);

        network::CurlHeader header;
        header.add("Content-Type: application/x-www-form-urlencoded");
        header.add(cookie);

        std::string param = "biliCSRF=" + m_cookies.cookie(domain).value(cookie_biliJct);

        post(PassportURL::Logout, response, param, header, false);
    }

    LogoutExitV2 logout;
    try
    {
        logout = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getLogoutExitV2 error: {}", e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }

    if (logout.code == 0)
    {
        BILIBILI_LOG_ERROR("getLogoutExitV2");
        std::lock_guard lk(m_mutexRequest);
        m_cookies = network::CurlCookies();
        m_commonOptions.erase(network::CookieFileds::opt);
    }

    return logout;
}

History BilibiliClient::getHistory(HistoryQueryParam param)
{
    std::string response;

    auto params = param.toParam();
    get(PassportURL::History, response, params);
    History history;
    try
    {
        history = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getHistory error! param: {} response: {}", param.toString(), e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return history;
}

std::string BilibiliClient::cookies() const
{
    return std::string(m_cookies);
}

void BilibiliClient::setCookies(std::string cookies)
{
    std::lock_guard lk(m_mutexRequest);
    m_cookies = network::CurlCookies(cookies);
    m_commonOptions[network::CookieFileds::opt] = std::make_shared<network::CookieFileds>(m_cookies.cookie(domain));
}

bool BilibiliClient::isLogined() const
{
    return !m_cookies.cookie(domain).value("SESSDATA").empty();
}

void BilibiliClient::resetWbi()
{
    const auto navData = getNavInfo();
    const auto key = navData.data.wbi_img;
    if (!key.img_url.empty() && !key.sub_url.empty())
    {
        const auto img_url = std::filesystem::path(util::utf8ToLocale(key.img_url)).stem().string();
        const auto sub_url = std::filesystem::path(util::utf8ToLocale(key.sub_url)).stem().string();
        nlohmann::json j;
        m_mixinKey.mixin_key = GetMixinKey(img_url + sub_url);
        m_mixinKey.Expires = (std::time(nullptr) + daySeconds) / daySeconds;
    }
}

void BilibiliClient::encodeWithWbi(ParamType& params)
{
    if (m_mixinKey.mixin_key.empty() || isExpired(m_mixinKey.Expires))
    {
        // 如果 mixinKey 过期或不存在，则从 Nav 中获取最新的 mixinKey
        BILIBILI_LOG_INFO("mixinKey expired or not exist, get from Nav");
        resetWbi();
    }

    // 添加 wts 字段
    const time_t curr_time = time(nullptr);
    params.emplace("wts", std::to_string(curr_time));

    // 按照 key 重排参数
    std::vector<std::string> sortedParams;
    for (auto& [key, value] : params)
    {
        // 过滤 value 中的 "!'()*" 字符
        const std::string filteredValue = filterCharacters(value);
        // url encode
        sortedParams.push_back(urlEncode(key) + "=" += urlEncode(filteredValue));
    }
    std::sort(sortedParams.begin(), sortedParams.end());
    // 序列化参数
    const std::string query = std::accumulate(std::next(sortedParams.begin()), sortedParams.end(), sortedParams[0], [](std::string a, std::string b) {
        return std::move(a) + '&' + std::move(b);
    });

    // 计算 w_rid 字段
    params.emplace("w_rid", MD5Hash(query + m_mixinKey.mixin_key));
}

nlohmann::json BilibiliClient::getDataFromRespones(const std::string& respones)
{
    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(respones);
        util::JsonProcess::removeNullValues(json);
    }
    catch (std::exception& e)
    {
        BILIBILI_LOG_ERROR("Error parsing response: {}", e.what());
    }

    return json;
}

network::CurlHeader BilibiliClient::passPortHeaders()
{
    network::CurlHeader header;

    std::string userAgent = std::string("user-agent: ") + network::chrome;
    header.add(userAgent);
    header.add(Headers::PassportReferer);

    return header;
}

void BilibiliClient::initDefaultHeaders()
{
    std::string userAgent = std::string("user-agent: ") + network::chrome;
    m_commonHeaders.add(userAgent);
    m_commonHeaders.add(network::accept_language);
    m_commonHeaders.add(network::accept_encoding);
}

void BilibiliClient::initDefaultOptions()
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
    auto verbose = std::make_shared<network::Verbose>(false);
    m_commonOptions.insert({verbose->getOption(), verbose});
}

}  // namespace biliapi
