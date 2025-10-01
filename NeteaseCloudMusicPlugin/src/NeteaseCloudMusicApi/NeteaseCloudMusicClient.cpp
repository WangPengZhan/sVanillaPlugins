#include <regex>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>

#include "NeteaseCloudMusicClient.h"

#include "NeteaseCloudMusicApi.h"
#include "NeteaseCloudMusicConstants.h"
#include "NeteaseCloudMusicUtils.h"
#include "NeteaseCloudMusicLog.h"
#include "Util/JsonProcess.h"
#include "NetWork/HeaderBodyResponseWrapper.h"
#include "NetWork/CurlCpp/CurlCookie.h"
#include "NetWork/CurlCpp/CurlOption.h"
#include "DeviceIDData.h"

namespace
{

constexpr char s_X_Real_IP[] = "X-Real-IP";
constexpr char s_X_Forwarded_For[] = "X-Forwarded-For";
constexpr char s_remember_me[] = "__remember_me";
constexpr char s_ntes_kaola_ad[] = "ntes_kaola_ad";
constexpr char s_ntes_nuid[] = "_ntes_nuid";
constexpr char s_ntes_nnid[] = "_ntes_nnid";
constexpr char s_WNMCID[] = "WNMCID";
constexpr char s_WEVNSM[] = "WEVNSM";
constexpr char s_osver[] = "osver";
constexpr char s_deviceId[] = "deviceId";
constexpr char s_os[] = "os";
constexpr char s_channel[] = "channel";
constexpr char s_appver[] = "appver";
constexpr char s_NMTID[] = "NMTID";
constexpr char s_MUSIC_U[] = "MUSIC_U";
constexpr char s_MUSIC_A[] = "MUSIC_A";
constexpr char s_csrf_token[] = "csrf_token";
constexpr char s_csrf[] = "__csrf";
constexpr char s_username[] = "username";

struct OsInfo
{
    std::string os;
    std::string appver;
    std::string osver;
    std::string channel;
};

const std::unordered_map<netease::OsType, OsInfo> s_OsInfoMap{
    {netease::OsType::Pc,      {"pc", "3.0.18.203152", "Microsoft-Windows-10-Professional-build-22631-64bit", "netease"}},
    {netease::OsType::Linux,   {"linux", "1.2.1.0428", "Deepin 20.9", "netease"}                                        },
    {netease::OsType::Android, {"android", "8.20.20.231215173437", "14", "xiaomi"}                                      },
    {netease::OsType::Ios,     {"iPhone OS", "9.0.90", "16.2", "distribution"}                                          },
};

const std::unordered_map<netease::CryptoType, std::unordered_map<netease::OsType, std::string>> s_UserAgentMap{
    {netease::CryptoType::WeApi,
     {{netease::OsType::Pc,
       "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36 Edg/124.0.0.0"}}},
    {netease::CryptoType::LinuxApi,
     {{netease::OsType::Linux, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3112.90 Safari/537.36"}}   },
    {netease::CryptoType::Api,
     {{netease::OsType::Pc,
       "ozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Safari/537.36 Chrome/91.0.4472.164 NeteaseMusicDesktop/3.0.18.203152"},
      {netease::OsType::Android, "NeteaseMusic/9.1.65.240927161425(9001065);Dalvik/2.1.0 (Linux; U; Android 14; 23013RK75C Build/UKQ1.230804.001)"},
      {netease::OsType::Ios, "NeteaseMusic 9.0.90/5038 (iPhone; iOS 16.2; zh_CN)"}}                                                           }
};

std::string chooseUserAgent(netease::CryptoType crypto, netease::OsType os = netease::OsType::Pc)
{
    if (!s_UserAgentMap.contains(crypto))
    {
        return "";
    }
    if (!s_UserAgentMap.at(crypto).contains(os))
    {
        return "";
    }
    return s_UserAgentMap.at(crypto).at(os);
}

std::string getDefaultReferHeader()
{
    return "Referer: " + netease::home;
}

std::string getUserAgentHeader(netease::CryptoType crypto, netease::OsType os)
{
    return "User-Agent: " + chooseUserAgent(crypto, os);
}

std::string preparePostData(const std::unordered_map<std::string, std::string>& data)
{
    std::string postFields;
    bool bFirst = true;
    for (const auto& [fst, snd] : data)
    {
        if (!bFirst)
        {
            postFields += "&";
        }
        postFields += netease::urlEncode(fst) + "=" + netease::urlEncode(snd);
        bFirst = false;
    }
    return postFields;
}

std::vector<std::string> s_deviceIDs;
void readDeviceID()
{
    std::istringstream iss(deviceDataText());
    std::string line;
    s_deviceIDs.clear();
    s_deviceIDs.reserve(2000);
    while (std::getline(iss, line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue;
        }
        s_deviceIDs.push_back(line);
    }
}
std::string getRandomDeviceID()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    if (s_deviceIDs.empty())
    {
        return {};
    }
    const int index = std::rand() % s_deviceIDs.size();
    return s_deviceIDs[index];
}

nlohmann::json getDataFromRespones(const std::string& respones)
{
    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(respones);
        util::JsonProcess::removeNullValues(json);
    }
    catch (std::exception& e)
    {
        std::string error = e.what();
    }

    return json;
}

}  // namespace

namespace netease
{

NeteaseCloudMusicClient::NeteaseCloudMusicClient()
{
    readDeviceID();
    generateData();
    m_option.cryptoType = CryptoType::WeApi;
    createCookies(m_option);
    registerAnonymous();
}

NeteaseCloudMusicClient& NeteaseCloudMusicClient::globalClient()
{
    static NeteaseCloudMusicClient client;
    return client;
}

LoginUnikey NeteaseCloudMusicClient::getLoginKey()
{
    const std::string url = home + "/weapi/" + Login::QRUnikey.substr(5);
    Option option;
    {
        std::shared_lock lk(m_mutexRequest);
        option = m_option;
    }
    network::CurlHeader headers = createHeaders(option);
    headers.add("Cookie: " + std::string(option.cookie.cookie(domain)));
    nlohmann::ordered_json data = nlohmann::ordered_json::object();
    data["type"] = 3;
    data[s_csrf_token] = "";
    std::string postFields = getPostFields(option.cryptoType, data);

    std::string response;
    post(url, response, postFields, headers, false);

    LoginUnikey ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
        NETEASE_LOG_WARN("getLoginKey error: {}", e.what());
    }

    return ret;
}

LoginStatus NeteaseCloudMusicClient::getLoginStatus(const std::string& key)
{
    const std::string url = home + "/weapi/" + Login::QRCheck.substr(5);
    Option option;
    {
        std::shared_lock lk(m_mutexRequest);
        option = m_option;
    }
    network::CurlHeader headers = createHeaders(option);
    headers.add("Cookie: " + std::string(option.cookie.cookie(domain)));
    nlohmann::ordered_json data = nlohmann::ordered_json::object();
    data["key"] = key;
    data["type"] = 3;
    data[s_csrf_token] = "";
    std::string postFields = getPostFields(option.cryptoType, data);

    network::ResponseHeaderAndBody response;
    post(url, response, postFields, headers, false);

    LoginStatus ret;
    try
    {
        ret = getDataFromRespones(response.body);
    }
    catch (const std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
        NETEASE_LOG_WARN("getLoginStatus error: {}", e.what());
    }

    auto header = parseHeader(response.header);
    if (header.end() != header.find(network::set_cookies) && ret.code == 803)
    {
        NETEASE_LOG_INFO("Login success!");
        std::lock_guard lk(m_mutexRequest);
        m_option.cookie = network::CurlCookies(header.at(network::set_cookies));
        m_option.bIsLogin = true;
    }

    return ret;
}

SongDetails NeteaseCloudMusicClient::getSongDetails(std::vector<uint64_t> ids)
{
    const std::string url = home + "/weapi/" + Content::SongDetail.substr(5);
    nlohmann::ordered_json data;
    nlohmann::ordered_json jsonIds = nlohmann::ordered_json::array();
    for (const auto& id : ids)
    {
        nlohmann::ordered_json jsonId;
        jsonId["id"] = id;
        jsonIds.push_back(jsonId);
    }
    data["c"] = jsonIds.dump();
    data[s_csrf_token] = "";

    Option option;
    {
        std::shared_lock lk(m_mutexRequest);
        option = m_option;
    }
    network::CurlHeader headers = createHeaders(option);
    headers.add("Cookie: " + std::string(option.cookie.cookie(domain)));
    std::string postFields = getPostFields(option.cryptoType, data);

    std::string response;
    post(url, response, postFields, headers, false);

    SongDetails ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
        NETEASE_LOG_WARN("getSongDetails error: {}", e.what());
    }

    return ret;
}

SongPlayUrl NeteaseCloudMusicClient::getSongPlayUrl(std::vector<uint64_t> ids, SoundLevel level)
{
    Option option;
    {
        std::shared_lock lk(m_mutexRequest);
        option = m_option;
    }

    const std::string url = home + "/weapi/" + Content::SongUrl.substr(5);
    auto cookie = option.cookie.cookie(domain);
    nlohmann::ordered_json jsonIds = ids;
    nlohmann::ordered_json data;
    data["ids"] = jsonIds.dump();
    data["level"] = soundLevelToString(level);
    data["encodeType"] = level == SoundLevel::Sky ? "c51" : "flac";
    data[s_csrf_token] = "";

    network::CurlHeader headers = createHeaders(option);
    headers.add("Cookie: " + std::string(cookie));
    std::string postFields = getPostFields(option.cryptoType, data);

    std::string response;
    post(url, response, postFields, headers, false);

    SongPlayUrl ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
        NETEASE_LOG_WARN("getSongPlayUrl error: {}", e.what());
    }

    return ret;
}

AblumDetails NeteaseCloudMusicClient::getAlbum(std::string id)
{
    Option option;
    {
        std::shared_lock lk(m_mutexRequest);
        option = m_option;
    }

    const std::string url = home + "/weapi/" + Content::Album.substr(5) + "/" + id;

    auto cookie = option.cookie.cookie(domain);
    nlohmann::ordered_json data;
    data[s_csrf_token] = "";

    network::CurlHeader headers = createHeaders(option);
    headers.add("Cookie: " + std::string(cookie));
    std::string postFields = getPostFields(option.cryptoType, data);

    std::string response;
    post(url, response, postFields, headers, false);

    AblumDetails ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
        NETEASE_LOG_WARN("getAlbum error: {}", e.what());
    }

    return ret;
}

PlaylistDetails NeteaseCloudMusicClient::getPlaylist(std::string id)
{
    Option option;
    {
        std::shared_lock lk(m_mutexRequest);
        option = m_option;
    }

    const std::string url = home + "/weapi/" + Content::PlaylistDetail.substr(5);
    auto cookie = option.cookie.cookie(domain);
    nlohmann::ordered_json data;
    data["id"] = id;
    data["n"] = 100000;
    data["s"] = 10000;
    data[s_csrf_token] = "";

    network::CurlHeader headers = createHeaders(option);
    headers.add("Cookie: " + std::string(cookie));
    std::string postFields = getPostFields(option.cryptoType, data);

    std::string response;
    post(url, response, postFields, headers, false);

    PlaylistDetails ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
        NETEASE_LOG_WARN("getAlbum error: {}", e.what());
    }

    return ret;
}

MVResponse NeteaseCloudMusicClient::getMVDetail(std::string id)
{
    Option option;
    {
        std::shared_lock lk(m_mutexRequest);
        option = m_option;
    }

    const std::string url = home + "/weapi/" + Content::MVDetail.substr(5);
    auto cookie = option.cookie.cookie(domain);
    nlohmann::ordered_json data;
    data["id"] = id;
    data[s_csrf_token] = "";

    network::CurlHeader headers = createHeaders(option);
    headers.add("Cookie: " + std::string(cookie));
    std::string postFields = getPostFields(option.cryptoType, data);

    std::string response;
    post(url, response, postFields, headers, false);

    MVResponse ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
        NETEASE_LOG_WARN("getAlbum error: {}", e.what());
    }

    return ret;
}

MVPlayUrl NeteaseCloudMusicClient::getMVPlayUrl(std::string id)
{
    Option option;
    {
        std::shared_lock lk(m_mutexRequest);
        option = m_option;
    }

    const std::string url = home + "/weapi/" + Content::MVUrl.substr(5);
    auto cookie = option.cookie.cookie(domain);
    nlohmann::ordered_json data;
    data["id"] = id;
    data["r"] = 1080;
    data[s_csrf_token] = "";

    network::CurlHeader headers = createHeaders(option);
    headers.add("Cookie: " + std::string(cookie));
    std::string postFields = getPostFields(option.cryptoType, data);

    std::string response;
    post(url, response, postFields, headers, false);
    std::ofstream ss("test.json");
    ss << response << std::endl;

    MVPlayUrl ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
        NETEASE_LOG_WARN("getAblum error: {}", e.what());
    }

    return ret;
}

network::CurlHeader NeteaseCloudMusicClient::createHeaders(const Option& option)
{
    network::CurlHeader headers;
    if (!option.realIP.empty())
    {
        headers.add(concatenateCurlHeader(s_X_Real_IP, option.realIP));
        headers.add(concatenateCurlHeader(s_X_Forwarded_For, option.realIP));
    }
    CryptoType cryptoType = option.cryptoType;
    if (cryptoType == CryptoType::Unknown)
    {
        cryptoType = CryptoType::EApi;
    }

    // headers.add
    switch (option.cryptoType)
    {
    case CryptoType::WeApi:
    {
        headers.add(getDefaultReferHeader());
        headers.add(getUserAgentHeader(cryptoType, OsType::Pc));
        break;
    }
    case CryptoType::LinuxApi:
    {
        break;
    }
    case CryptoType::EApi:
        break;
    case CryptoType::Api:
        break;
    case CryptoType::Unknown:
        break;
    default:
        break;
    }
    return headers;
}

void NeteaseCloudMusicClient::createCookies(Option& option)
{
    if (option.cookie.contains(domain))
    {
        return;
    }

    const std::string _ntes_nuid = generateRandomString(32, Encrypt::BASE62);
    const auto now = std::chrono::system_clock::now();
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    network::CurlCookie cookie;
    cookie.setDomain(domain);
    cookie.setName(s_remember_me).setValue("true");
    cookie.setName(s_ntes_kaola_ad).setValue("1");
    cookie.setName(s_ntes_nuid).setValue(_ntes_nuid);
    cookie.setName(s_ntes_nnid).setValue(_ntes_nuid + "," + std::to_string(now_ms));
    cookie.setName(s_WNMCID).setValue(generateWNMCID());
    cookie.setName(s_WEVNSM).setValue("1.0.0");
    cookie.setName(s_osver).setValue(s_OsInfoMap.at(option.os).osver);
    cookie.setName(s_deviceId).setValue(m_deviceId);
    cookie.setName(s_os).setValue(s_OsInfoMap.at(option.os).os);
    cookie.setName(s_channel).setValue(s_OsInfoMap.at(option.os).channel);
    cookie.setName(s_appver).setValue(s_OsInfoMap.at(option.os).appver);

    if (!option.bIsLogin)
    {
        cookie.setName(s_NMTID).setValue(generateRandomString(16, Encrypt::BASE62));
    }

    if (!cookie.contains(s_MUSIC_U) && !cookie.contains(s_MUSIC_A))
    {
        cookie.setName(s_MUSIC_A).setValue("");
    }
    option.cookie.addCurlCookie(cookie);
}

std::string NeteaseCloudMusicClient::getAnonimousEncodeID()
{
    const std::string deviceID = getRandomDeviceID();
    m_deviceId = deviceID;
    const std::string digestHex = cloudMusicDllEncodeID(deviceID);
    return base64Encode(deviceID + " " + digestHex);
}

void NeteaseCloudMusicClient::generateData(bool force)
{
    if (m_data.empty() || force)
    {
        m_data = nlohmann::ordered_json::object();
        m_data[s_username] = getAnonimousEncodeID();
        m_data[s_csrf_token] = "";
    }
}

std::string NeteaseCloudMusicClient::getPostFields(CryptoType cryptoType, const nlohmann::ordered_json& data)
{
    std::unordered_map<std::string, std::string> encryptData;
    switch (cryptoType)
    {
    case netease::CryptoType::WeApi:
        encryptData = weapi(data);
        break;
    case netease::CryptoType::EApi:
        break;
    case netease::CryptoType::LinuxApi:
        break;
    case netease::CryptoType::Api:
        break;
    case netease::CryptoType::Unknown:
        break;
    default:
        break;
    }

    return preparePostData(encryptData);
}

void NeteaseCloudMusicClient::registerAnonymous()
{
    Option option;
    {
        std::shared_lock lk(m_mutexRequest);
        option = m_option;
    }

    network::CurlHeader headers = createHeaders(option);
    headers.add("Cookie: " + std::string(option.cookie.cookie(domain)));

    const std::string url = home + "/weapi/" + Login::Anonimous.substr(5);
    network::ResponseHeaderAndBody response;
    std::string postFields = getPostFields(option.cryptoType, m_data);
    post(url, response, postFields, headers, false);
    auto header = parseHeader(response.header);
    if (header.end() != header.find(network::set_cookies))
    {
        NETEASE_LOG_INFO("Register Anonimous success!");
        std::lock_guard lk(m_mutexRequest);
        m_option.cookie.addCurlCookie(network::CurlCookie(header.at(network::set_cookies)));
        m_option.bIsLogin = false;
    }
}

std::string NeteaseCloudMusicClient::cookies() const
{
    std::shared_lock lk(m_mutexRequest);
    return std::string(m_option.cookie);
}

void NeteaseCloudMusicClient::setCookies(std::string cookies)
{
    std::lock_guard lk(m_mutexRequest);
    m_option.cookie = network::CurlCookies(cookies);
    m_option.bIsLogin = m_option.cookie.cookie(domain).contains(s_csrf);
}

bool NeteaseCloudMusicClient::isLogined() const
{
    std::shared_lock lk(m_mutexRequest);
    return m_option.bIsLogin;
}

}  // namespace netease
