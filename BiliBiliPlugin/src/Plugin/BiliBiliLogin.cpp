#include "BiliBiliLogin.h"

#include <BaseVideoView.h>

#include <fstream>
#include <mutex>

#include "BiliBiliResource.h"
#include "BiliApi/BilibiliClient.h"
#include "BiliApi/BiliApiConstants.h"
#include "Util/TimerUtil.h"
#include "BiliBiliPlugin.h"
#include "Util/LocaleHelper.h"
#include "Util/QrCodeGenerator.h"
#include "BiliBiliPluginMessage.h"
#include "Convert.h"

namespace
{

std::string vipTypeToString(int type)
{
    switch (type)
    {
    case 0:
        return "无";
    case 1:
        return "月度大会员";
    case 2:
        return "年度大会员";
    default:
        return "未知类型";
    }
}

}  // namespace

BiliBiliLogin::LoginResource BiliBiliLogin::m_biliRes{qrc_background, qrc_loading, qrc_tip, qrc_waitConfirm, qrc_complete, qrc_init, qrc_refresh};

BiliBiliLogin::LoginStatus BiliBiliLogin::getLoginStatus()
{
    std::string qrcodeKey;
    {
        std::lock_guard lk(m_mutex);
        qrcodeKey = m_qrcodeKey;
    }

    const auto loginStatus = biliapi::BilibiliClient::globalClient().getLoginStatus(qrcodeKey);

    if (loginStatus.code == 0)
    {
        if (loginStatus.data.code == 86038)
        {
            return Timeout;
        }
        else if (loginStatus.data.code == 86101)
        {
            return NoScan;
        }
        else if (loginStatus.data.code == 86090)
        {
            return ScannedNoAck;
        }
        else if (loginStatus.data.code == 0)
        {
            return Success;
        }
        else
        {
            return Error;
        }
    }
    else
    {
        return Error;
    }
}

bool BiliBiliLogin::getScanContext(std::string& content)
{
    const auto login = biliapi::BilibiliClient::globalClient().getLoginUrl();
    if (login.data.qrcode_key.empty() || login.code != 0)
    {
        return false;
    }

    std::string path = BiliBiliPlugin::getDir() + "login/bilibili_qrc.svg";
    if (!std::filesystem::exists(std::filesystem::path(path).parent_path()))
    {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }

    std::ofstream file(path);
    if (!file)
    {
        return false;
    }

    file << QrCodeGenerator::generateQR(login.data.url);
    file.close();

    {
        std::lock_guard lk(m_mutex);
        m_qrcodeKey = login.data.qrcode_key;
    }

    content = util::localeToUtf8(path);
    return true;
}

void BiliBiliLogin::loginSuccess()
{
}

UserInfo BiliBiliLogin::getUserInfo(std::string dir)
{
    UserInfo userInfo;

    const auto nav = biliapi::BilibiliClient::globalClient().getNavInfo();
    if (nav.code != 0)
    {
        return userInfo;
    }
    userInfo.uname = nav.data.uname;
    userInfo.id = std::to_string(nav.data.mid);
    userInfo.vipType = vipTypeToString(nav.data.vipType);
    userInfo.home = biliplugin::domain;

    if (!nav.data.face.empty())
    {
        if (!dir.empty() && !std::filesystem::exists(dir))
        {
            std::filesystem::create_directories(dir);
        }

        std::string path = dir + "/" + std::to_string(nav.data.mid) + ".jpg";
        if (network::downloadFileChecked(biliapi::BilibiliClient::globalClient(), nav.data.face, path))
        {
            userInfo.facePath = util::localeToUtf8(path);
        }
    }

    return userInfo;
}

bool BiliBiliLogin::supportsLogin() const
{
    return true;
}

std::string BiliBiliLogin::cookies() const
{
    return biliapi::BilibiliClient::globalClient().cookies();
}

void BiliBiliLogin::setCookies(std::string cookies)
{
    biliapi::BilibiliClient::globalClient().setCookies(cookies);
}

bool BiliBiliLogin::refreshCookies(std::string cookies)
{
    return false;
}

bool BiliBiliLogin::isLoggedIn() const
{
    return biliapi::BilibiliClient::globalClient().isLogined();
}

bool BiliBiliLogin::logout()
{
    const auto logoutInfo = biliapi::BilibiliClient::globalClient().getLogoutExitV2();
    return logoutInfo.code == 0;
}

std::string BiliBiliLogin::domain() const
{
    return biliapi::domain;
}

std::vector<adapter::BaseVideoView> BiliBiliLogin::history()
{
    biliapi::HistoryQueryParam param;
    param.ps = 30;
    const auto historyInfo = biliapi::BilibiliClient::globalClient().getHistory(param);

    return convertVideoView(historyInfo);
}

const BiliBiliLogin::LoginResource& BiliBiliLogin::allResources() const
{
    return m_biliRes;
}

const std::vector<uint8_t>& BiliBiliLogin::resource(ResourceIndex index) const
{
    if (index >= 0 && index < m_biliRes.size())
    {
        return m_biliRes.at(index);
    }

    return m_emptyString;
}

int BiliBiliLogin::pluginId() const
{
    return biliplugin::pluginID;
}
