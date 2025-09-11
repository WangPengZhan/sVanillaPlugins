#pragma once

#include <string>
#include <atomic>
#include <vector>
#include <shared_mutex>

#include <nlohmann/json.hpp>

#include <PluginConfig.h>

#include "NetWork/CNetWork.h"
#include "NetWork/CurlCpp/CurlCookieJar.h"
#include "NeteaseCloudMusicApi.h"

namespace netease
{

struct Option
{
    CryptoType cryptoType{CryptoType::Api};
    OsType os{OsType::Ios};
    network::CurlCookies cookie;
    std::string realIP;
    std::string e_r;
    bool bIsLogin{false};
};

class PLUGIN_PORT NeteaseCloudMusicClient : public network::NetWork
{
public:
    NeteaseCloudMusicClient();
    ~NeteaseCloudMusicClient() = default;

    static NeteaseCloudMusicClient& globalClient();

    LoginUnikey getLoginKey();
    LoginStatus getLoginStatus(const std::string& key);
    void registerAnonymous();

    SongDetails getSongDetails(std::vector<uint64_t> ids);
    SongPlayUrl getSongPlayUrl(std::vector<uint64_t> ids, SoundLevel level = SoundLevel::Standard);
    AblumDetails getAlbum(std::string id);
    PlaylistDetails getPlaylist(std::string id);
    MVResponse getMVDetail(std::string id);
    MVPlayUrl getMVPlayUrl(std::string ids);

    std::string cookies() const;
    void setCookies(std::string cookies);

    [[nodiscard]] bool isLogined() const;

private:
    network::CurlHeader createHeaders(const Option& option);
    void createCookies(Option& option);

    std::string getAnonimousEncodeID();
    void generateData(bool force = false);
    std::string getPostFields(CryptoType cryptoType, const nlohmann::ordered_json& data);

private:
    mutable std::shared_mutex m_mutexRequest;
    nlohmann::ordered_json m_data;
    std::string m_deviceId;
    Option m_option;
};
}  // namespace netease
