#include "DouYinLogin.h"

#include <BaseVideoView.h>
#include <fstream>

#include "DouYinResource.h"
#include "DouYinApi/DouYinClient.h"
#include "DouYinApi/DouYinConstants.h"
#include "Util/TimerUtil.h"
#include "Util/LocaleHelper.h"
#include "DouYinPlugin.h"
#include "DouYinPluginMessage.h"
#include "Convert.h"

namespace
{
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

inline bool is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::vector<unsigned char> base64_decode(const std::string& encoded_string)
{
    int in_len = encoded_string.size();
    int i = 0, j = 0, in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;
    ret.reserve(encoded_string.size());

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; j < i - 1; j++)
            ret.push_back(char_array_3[j]);
    }

    return ret;
}
}  // namespace

DouYinLogin::LoginResource DouYinLogin::m_douyinRes{qrc_background, qrc_loading, qrc_tip, qrc_waitConfirm, qrc_complete, qrc_init, qrc_refresh};

DouYinLogin::DouYinLogin()
    : AbstractLoginApi()
    , m_client(douyinapi::DouYinClient::globalClient())
{
}

DouYinLogin::LoginStatus DouYinLogin::getLoginStatus()
{
    std::string token;
    {
        std::lock_guard lc(m_mutexData);
        token = m_token;
    }

    douyinapi::QRCodeStatusResponse status = m_client.getLoginStatus(token);
    if (status.data.error_code != 0)
    {
        return LoginStatus::Unknow;
    }

    if (status.data.status == "new")
    {
        return LoginStatus::NoScan;
    }
    else if (status.data.status == "scanned" || status.data.status == "confirm" || status.data.status == "scan")
    {
        return LoginStatus::ScannedNoAck;
    }
    else if (status.data.status == "expired")
    {
        return LoginStatus::Timeout;
    }
    else if (status.data.status == "success")
    {
        return LoginStatus::Success;
    }

    return LoginStatus::Error;
}

bool DouYinLogin::getScanContext(std::string& content)
{
    douyinapi::QRCodeResponse qrc = m_client.getQRCode();

    {
        std::lock_guard lc(m_mutexData);
        m_token = qrc.data.token;
    }

    std::string path = douyinplugin::DouYinPlugin::getDir() + "login/douyin_qrc.png";
    if (!std::filesystem::exists(std::filesystem::path(path).parent_path()))
    {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }

    std::string base_content = qrc.data.qrcode;
    auto bin = base64_decode(base_content);
    std::ofstream qrFile(path, std::ios::binary);
    auto strBin = std::string(bin.begin(), bin.end());
    qrFile << strBin;
    content = util::localeToUtf8(path);

    return true;
}

void DouYinLogin::loginSuccess()
{
}

UserInfo DouYinLogin::getUserInfo(std::string dir)
{
    douyinapi::FollowingResponse following = m_client.getFollowing();
    douyinapi::FollowingUser user = ::getUserInfo(following);
    if (user.sec_uid.empty())
    {
        return {};
    }

    UserInfo userInfo;
    userInfo.id = user.uid;
    userInfo.option1 = user.sec_uid;
    userInfo.uname = user.nickname;
    userInfo.sign = user.signature;

    if (!user.avatar_thumb.url_list.empty())
    {
        userInfo.facePath = dir + "/" + userInfo.id + ".jpg";
        m_client.downloadImage(user.avatar_thumb.url_list[0], userInfo.facePath);
        userInfo.facePath = util::localeToUtf8(userInfo.facePath);
    }

    return userInfo;
}

bool DouYinLogin::supportsLogin() const
{
    return true;
}

std::string DouYinLogin::cookies() const
{
    return m_client.cookies();
}

void DouYinLogin::setCookies(std::string cookies)
{
    m_client.setCookies(cookies);
}

bool DouYinLogin::refreshCookies(std::string cookies)
{
    return false;
}

bool DouYinLogin::isLoggedIn() const
{
    return m_client.isLogined();
}

bool DouYinLogin::logout()
{
    return true;
}

std::string DouYinLogin::domain() const
{
    return douyinapi::domain;
}

std::vector<adapter::BaseVideoView> DouYinLogin::history()
{
    std::vector<adapter::BaseVideoView> views;
    int cursor = 0;
    douyinapi::SeriesDetail detail;

    do
    {
        detail = m_client.getUserHistory(cursor, 10);
        auto batch = convertSeriesDetail(detail);
        views.insert(views.end(), batch.begin(), batch.end());
        cursor = detail.max_cursor;
    } while (detail.has_more);

    return views;
}

const DouYinLogin::LoginResource& DouYinLogin::allResources() const
{
    return m_douyinRes;
}

const std::vector<uint8_t>& DouYinLogin::resource(ResourceIndex index) const
{
    if (index >= 0 && index < m_douyinRes.size())
    {
        return m_douyinRes.at(index);
    }

    return m_emptyString;
}

int DouYinLogin::pluginId() const
{
    return douyinplugin::pluginID;
}
