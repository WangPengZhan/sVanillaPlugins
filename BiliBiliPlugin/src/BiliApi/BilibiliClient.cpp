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

std::string BilibiliClient::m_cookieTicket;

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

VideoViewOrigin BilibiliClient::getVideoView(const std::string& bvid, IDType type)
{
    if (type != IDType::Aid && type != IDType::Bid)
    {
        return {};
    }

    std::string idType = type == IDType::Aid ? "aid" : "bvid";
    ParamType param;
    param.emplace(idType, bvid);

    std::string response;
    get(VideoURL::View, response, param);
    VideoViewOrigin ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getVideoView error! {}: {}, error: {}", idType, bvid, e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

BangumiInfo BilibiliClient::getSeasonVideoView(const std::string& id, IDType type)
{
    if (type != IDType::BangumiSS && type != IDType::BangumiEP)
    {
        return {};
    }

    std::string idType = type == IDType::BangumiSS ? "season_id" : "ep_id";
    ParamType param;
    param.emplace(idType, id);

    std::string response;
    get(VideoURL::PgcSeason, response, param);
    
    BangumiInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getSeasonVideoView error! {}: {}, error: {}", idType, id, e.what());
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
        BILIBILI_LOG_ERROR("getPlayUrl error! cid: {},  qn: {},  bvid: {}, fnval: {}, error: {}", cid, qn, bvid, fnval, str);
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

MediaInfoResponse BilibiliClient::getMdVideoView(const std::string& id)
{
    ParamType param;
    param.emplace("media_id", id);

    std::string response;
    get(VideoURL::MdSeason, response, param);

    MediaInfoResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getMdVideoView error! id: {}, error: {}", id, e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

UgcPlayUrlResponse BilibiliClient::getPlayUrl(long long id, IDType type, long long qn, long long fnval)
{
    if (type != IDType::Cid && type != IDType::BangumiEP)
    {
        return {};
    }

    std::string idType = type == IDType::BangumiEP ? "ep_id" : "cid";
    ParamType param;
    param.emplace(idType, std::to_string(id));
    param.emplace("qn", std::to_string(qn));
    param.emplace("fnver", "0");
    param.emplace("fnval", std::to_string(fnval));
    param.emplace("fourk", "1");

    encodeWithWbi(param);

    std::string response;
    get(VideoURL::PgcPlayUrl, response, param);

    UgcPlayUrlResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::string str = e.what();
        BILIBILI_LOG_ERROR("getPlayUrl error! {}: {},  qn: {}, fnval: {}, error: {}", idType, id, qn, fnval, str);
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

CheeseInfoResponse BilibiliClient::getCheeseVideoView(const std::string& id, IDType type)
{
    if (type != IDType::CheeseSS && type != IDType::CheeseEP)
    {
        return {};
    }

    std::string idType = type == IDType::CheeseSS ? "season_id" : "ep_id";
    ParamType param;
    param.emplace(idType, id);

    std::string response;
    get(VideoURL::CheeseSeason, response, param);

    CheeseInfoResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getCheeseVideoView error! {}: {}, error: {}", idType, id, e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

CheesePlayUrlResponse BilibiliClient::getPlayUrl(long long avid, long long ep_id, long long cid, long long qn, long long fnval)
{
    ParamType param;
    param.emplace("avid", std::to_string(avid));
    param.emplace("ep_id", std::to_string(ep_id));
    param.emplace("cid", std::to_string(cid));
    param.emplace("qn", std::to_string(qn));
    param.emplace("fnver", "0");
    param.emplace("fnval", std::to_string(fnval));
    param.emplace("fourk", "1");

    encodeWithWbi(param);

    std::string response;
    get(VideoURL::CheesePlayUrlApi, response, param);

    CheesePlayUrlResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::string str = e.what();
        BILIBILI_LOG_ERROR("getPlayUrl error! avid: {},  ep_id: {}, cid: {}, qn: {}, fnval: {}, error: {}", avid, ep_id, cid, qn, fnval, str);
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

FavDetailInfo BilibiliClient::getFavDetail(const std::string& media_id)
{
    ParamType param;
    param.emplace("media_id", media_id);
    param.emplace("platform", "web");

    std::string response;
    get(VideoURL::FavInfoDetail, response, param);

    FavDetailInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getCreatedFavList error! media_id: {}, error: {}", media_id, e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

FavVideoInfoResponse BilibiliClient::getFavVideoInfo(const std::vector<FavItemInfo> ids, int folder_mid, int folder_id)
{
    std::string resources;
    for (const auto& id : ids)
    {
        auto resource = std::to_string(id.id) + ":" + std::to_string(id.type) + ",";
        resources += resource;
    }

    if (!resources.empty())
    {
        resources.pop_back();
    }

    ParamType param;
    param.emplace("resources", resources);
    // param.emplace("folder_mid", std::to_string(folder_mid));
    // param.emplace("folder_id", std::to_string(folder_id));
    param.emplace("platform", "web");

    std::string response;
    get(VideoURL::FavVideoInfo, response, param);

    FavVideoInfoResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getCreatedFavList error! folder_mid: {}, folder_id:{} error: {}", folder_mid, folder_id, e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

FavListInfo BilibiliClient::getCreatedFavList(int ps, int pn, int up_mid)
{
    ParamType param;
    param.emplace("ps", std::to_string(ps));
    param.emplace("pn", std::to_string(pn));
    param.emplace("up_mid", std::to_string(up_mid));
    param.emplace("platform", "web");

    std::string response;
    get(VideoURL::CreatedFavList, response, param);

    FavListInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getCreatedFavList error! up_mid: {}, error: {}", up_mid, e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

FavListInfo BilibiliClient::getCollectFavList(int ps, int pn, int up_mid)
{
    ParamType param;
    param.emplace("ps", std::to_string(ps));
    param.emplace("pn", std::to_string(pn));
    param.emplace("up_mid", std::to_string(up_mid));
    param.emplace("platform", "web");

    std::string response;
    get(VideoURL::CollectedFavList, response, param);

    FavListInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getCreatedFavList error! up_mid: {}, error: {}", up_mid, e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

FavDataResponse BilibiliClient::getFavInfo(const std::string& media_id)
{
    ParamType param;
    param.emplace("media_id", media_id);

    network::CurlHeader headers;
    headers.add("accept: */*");
    headers.add("Accept-Encoding: gzip, deflate, br, zstd");
    headers.add("Accept-Language: zh-CN,zh;q=0.9");
    std::string userAgent = std::string("user-agent: ") + network::chrome;
    headers.add(userAgent);
    headers.add(Headers::DefaultReferer);
    headers.add("origin: https://www.bilibili.com");

    std::string response;
    get(VideoURL::FavInfoUrl, response, param, headers, false);

    FavDataResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getVideoView error! bvid: {}, error: {}", media_id, e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

VideoWorksResponse BilibiliClient::getUserVideoWroks(const std::string& mid)
{
    ParamType param;
    param.emplace("mid", mid);
    param.emplace("order", "pubdate");
    param.emplace("ps", "50");
    param.emplace("tid", "0");
    param.emplace("pn", "1");
    param.emplace("index", "0");
    param.emplace("keyword", "");
    param.emplace("order_avoided", "true");
    param.emplace("platform", "web");
    param.emplace("web_location", "333.1387");
    param.emplace("dm_img_list", "[]");
    param.emplace("dm_img_str", "V2ViR0wgMS4wIChPcGVuR0wgRVMgMi4wIENocm9taXVtKQ");
    param.emplace("dm_cover_img_str", "QU5HTEUgKE5WSURJQSwgTlZJRElBIEdlRm9yY2UgUlRYIDQwNjAgTGFwdG9wIEdQVSAoMHgwMDAwMjhFMCkgRGlyZWN0M0QxMSB2c181XzAgcHNfNV8wLCBEM0QxMSlHb29nbGUgSW5jLiAoTlZJRElBKQ");
    param.emplace("dm_img_inter", "{\"ds\":[],\"wh\":[5231,6067,75],\"of\":[475,950,475]}");
    encodeWithWbi(param);

    network::CurlHeader headers;
    std::string userAgent = std::string("user-agent: ") + network::chrome;
    headers.add(userAgent);
    headers.add(network::accept_language);
    headers.add(network::accept_encoding);
    headers.add("referer: https://space.bilibili.com/" + mid + "/upload/video");
    headers.add("origin: https://space.bilibili.com");

    std::string response;
    get(VideoURL::UserVideoUrl, response, param, headers, false);
    VideoWorksResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("getVideoView error! mid: {}, error: {}", mid, e.what());
        BILIBILI_LOG_ERROR("response is {}", response);
    }
    return ret;
}

BiliTicketResponse BilibiliClient::getTicket()
{
    long ts = std::time(nullptr);

    std::string key = "XgwSnGZ1p";
    std::string message = "ts" + std::to_string(ts);
    std::string hexsign = hmac_sha256(key, message);

    ParamType params;
    params["key_id"] = "ec02";
    params["hexsign"] = hexsign;
    params["context[ts]"] = std::to_string(ts);
    params["csrf"] = "";
    std::string paramstr = paramsString(params);

    network::CurlHeader headers;
    std::string userAgent = std::string("user-agent: ") + network::chrome;
    headers.add(userAgent);

    std::string response;
    post(ticketUrl + "?" + paramstr, response, headers, false);

    BiliTicketResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("response is {}", response);
    }

    return ret;
}

BuvidInfoResponse BilibiliClient::getBuvidInfo()
{
    std::string response;
    get(buvidUrl, response);

    BuvidInfoResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        BILIBILI_LOG_ERROR("response is {}", response);
    }

    return ret;
}

Buvid34InfoResponse BilibiliClient::getBuvid34Info()
{
    network::CurlHeader headers;
    std::string userAgent = std::string("user-agent: ") + network::chrome;
    headers.add(userAgent);
    headers.add("accept: application/json, text/plain, */*");
    headers.add(network::accept_language);
    headers.add(network::accept_encoding);
    headers.add(network::connect_type);

    std::string response;
    get(buvid34Url, response, headers, false);

    Buvid34InfoResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
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
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
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
        m_commonOptions.erase(network::CookieFields::opt);
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
    m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
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
    m_commonHeaders.add(Headers::DefaultReferer);
    m_commonHeaders.add("origin: https://www.bilibili.com");
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
