#pragma once
#include <string>

#include "BiliApi.h"
#include "BilibiliUrl.h"
#include "NetWork/CNetWork.h"
#include "NetWork/CurlCpp/CurlCookieJar.h"

namespace biliapi
{

enum class BusinessType
{
    Archive,
    Pgc,
    Live,
    Article_list,
    Article,

    Unkown = 255,
};

std::string to_string(BusinessType businessType);

enum HistoryType
{
    All,
    Archive,
    Live,
    Article,
    Unkown = 255,
};

std::string to_string(HistoryType historyType);

struct HistoryQueryParam
{
    int max{};
    BusinessType business{BusinessType::Unkown};
    uint64_t view_at{};
    HistoryType type{HistoryType::Unkown};
    int ps{};

    network::NetWork::ParamType toParam() const;
    std::string toString() const;
};

class BilibiliClient : public network::NetWork
{
public:
    BilibiliClient();
    ~BilibiliClient() = default;

    static BilibiliClient& globalClient();

    VideoViewOrigin getVideoView(const std::string& bvid, IDType type);
    PlayUrlOrigin getPlayUrl(long long cid, long long qn, const std::string& bvid, long long fnval);

    BangumiInfo getSeasonVideoView(const std::string& id, IDType type);
    MediaInfoResponse getMdVideoView(const std::string& id);
    UgcPlayUrlResponse getPlayUrl(long long id, IDType type, long long qn, long long fnval);

    CheeseInfoResponse getCheeseVideoView(const std::string& id, IDType type);
    CheesePlayUrlResponse getPlayUrl(long long avid, long long ep_id, long long cid, long long qn, long long fnval);

    FavDetailInfo getFavDetail(const std::string& media_id);
    FavVideoInfoResponse getFavVideoInfo(const std::vector<FavItemInfo> ids, int folder_mid, int folder_id);
    FavListInfo getCreatedFavList(int ps, int pn, int up_mid);
    FavListInfo getCollectFavList(int ps, int pn, int up_mid);
    FavDataResponse getFavInfo(const std::string& media_id);

    VideoWorksResponse getUserVideoWroks(const std::string& mid);

    BiliTicketResponse getTicket();
    BuvidInfoResponse getBuvidInfo();
    Buvid34InfoResponse getBuvid34Info();

    LoginUrlOrigin getLoginUrl();
    LoginStatusScanning getLoginStatus(const std::string& qrcodeKey);
    Nav getNavInfo();
    LogoutExitV2 getLogoutExitV2();
    History getHistory(HistoryQueryParam param);

    std::string cookies() const;
    void setCookies(std::string cookies);

    [[nodiscard]] bool isLogined() const;

    void resetWbi();
    void encodeWithWbi(ParamType& params);

    static nlohmann::json getDataFromRespones(const std::string& respones);

    static network::CurlHeader passPortHeaders();

protected:
    void initDefaultHeaders();
    void initDefaultOptions();

private:
    network::CurlCookies m_cookies;  // we sellect this mode for debug
    MixinKey m_mixinKey;
    static std::string m_cookieTicket;
};

}  // namespace biliapi
