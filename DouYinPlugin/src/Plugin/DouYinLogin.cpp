#include "DouYinLogin.h"

#include <BaseVideoView.h>
#include <fstream>

#include "DouYinResource.h"
#include "DouYinApi/DouYinClient.h"
#include "DouYinApi/DouYinConstants.h"
#include "PluginCrypto/Encoding.h"
#include "Util/TimerUtil.h"
#include "Util/LocaleHelper.h"
#include "DouYinPlugin.h"
#include "DouYinPluginMessage.h"
#include "Convert.h"

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
    auto bin = encoding::base64Decode(base_content);
    std::ofstream qrFile(path, std::ios::binary);
    qrFile << bin;
    content = util::localeToUtf8(path);

    return true;
}

void DouYinLogin::loginSuccess()
{
}

UserInfo DouYinLogin::getUserInfo(std::string dir)
{
    const auto account = m_client.getAccountInfo();
    const auto& user = account.data;
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
    m_client.setCookies(std::move(cookies));
    return m_client.isLogined();
}

bool DouYinLogin::isLoggedIn() const
{
    return m_client.isLogined();
}

bool DouYinLogin::logout()
{
    m_client.clearSession();
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
        if (detail.status_code != 0 || detail.aweme_list.empty())
        {
            break;
        }
        auto batch = convertSeriesDetail(detail);
        views.insert(views.end(), batch.begin(), batch.end());
        const int nextCursor = detail.max_cursor;
        if (detail.has_more && nextCursor == cursor)
        {
            break;
        }
        cursor = nextCursor;
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
