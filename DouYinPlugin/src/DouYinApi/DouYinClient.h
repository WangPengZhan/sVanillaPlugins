#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <NetWork/CNetWork.h>
#include <NetWork/CurlCpp/CurlCookies.h>

#include "DouYinApi.h"

namespace douyinapi
{

class DouYinClient : public network::NetWork
{
public:
    DouYinClient();
    ~DouYinClient() = default;

    static DouYinClient& globalClient();

    QRCodeResponse getQRCode();

    AwemeDetailResponse getAwemeDetail(const std::string& awemeId);
    SeriesDetail getSeriesDetail(const std::string& seriesId, int cursor, int count);
    SeriesDetail getMixDetail(const std::string& mixId, int cursor, int count);
    SeriesDetail getUserAll(const std::string& userId, int cursor, int count);
    SeriesDetail getUserHistory(int cursor, int count);
    SeriesDetail getUserCollection(int cursor, int count);

    QRCodeStatusResponse getLoginStatus(const std::string& token, const std::string& captcha = "");
    AccountInfoResponse getAccountInfo();
    FollowingResponse getFollowing();

    bool isLogined() const;
    std::string cookies() const;
    void setCookies(std::string cookies);

    bool downloadImage(const std::string& url, const std::filesystem::path& path);

private:
    void initDefaultOptions();

    void registerAnonymous();
    void requestHome();
    void registerTtWid();
    void getMsToken();
    void getWebId();

private:
    static network::CurlHeader createLoginHeaders();
    static network::CurlHeader createDetailHeaders();
    static network::CurlHeader createParamHeaders();
    network::CurlHeader createCollectionHeaders();
    static std::string encodeData(const ParamType& params);
    static std::string encryptionParams(const ParamType& params, const std::string& method = "GET");

    static nlohmann::json getDataFromRespones(const std::string& respones);

private:
    network::CurlCookies m_cookies;
    std::string m_userId;
    std::string m_webId;
    std::string m_msToken;
};

}  // namespace douyinapi
