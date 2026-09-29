#include "WeiboLogin.h"

#include <utility>

#include <BaseVideoView.h>

#include "WeiboResource.h"
#include "WeiboApi/WeiboClient.h"
#include "WeiboApi/WeiboApiConstants.h"
#include "WeiboApi/WeiboLog.h"
#include "Util/TimerUtil.h"
#include "Util/LocaleHelper.h"
#include "WeiboPlugin.h"
#include "WeiboPluginMessage.h"

WeiboLogin::LoginResource WeiboLogin::m_weiboRes{qrc_background, qrc_loading, qrc_tip, qrc_waitConfirm, qrc_complete, qrc_init, qrc_refresh};

WeiboLogin::WeiboLogin()
    : AbstractLoginApi()
    , m_client(weiboapi::WeiboClient::globalClient())
{
}

WeiboLogin::LoginStatus WeiboLogin::getLoginStatus()
{
    std::string qrid;
    {
        std::lock_guard<std::mutex> lock(m_mutexData);
        qrid = m_qrid;
    }

    const auto loginStatus = m_client.getLoginStatus(qrid);

    if (loginStatus.retcode == 50114003)
    {
        return Timeout;
    }
    else if (loginStatus.retcode == 50114001)
    {
        return NoScan;
    }
    else if (loginStatus.retcode == 50114002)
    {
        return ScannedNoAck;
    }
    else if (loginStatus.retcode == 20000000)
    {
        {
            std::lock_guard lock(m_mutexData);
            m_alt = loginStatus.data.alt;
        }

        return Success;
    }
    else
    {
        return Error;
    }
}

bool WeiboLogin::getScanContext(std::string& content)
{
    auto qrc = m_client.getLoginUrl();
    std::string path = WeiboPlugin::getDir() + "login/weibo_qrc.png";
    if (!std::filesystem::exists(std::filesystem::path(path).parent_path()))
    {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }

    bool ret = m_client.getQrcImage("https:" + qrc.data.image, path);
    {
        std::lock_guard<std::mutex> lock(m_mutexData);
        m_qrid = qrc.data.qrid;
    }
    content = util::localeToUtf8(path);

    return ret;
}

void WeiboLogin::loginSuccess()
{
    std::string alt;
    {
        std::lock_guard<std::mutex> lock(m_mutexData);
        alt = m_alt;
    }

    auto loginInfo = m_client.loginWeibo(alt);
    m_client.crossDomainRequest(loginInfo.crossDomainUrlList);
    if (!m_client.isLogined())
    {
        WEIBO_LOG_ERROR("login did not establish a session, retcode: {}", loginInfo.retcode);
    }
}

UserInfo WeiboLogin::getUserInfo(std::string dir)
{
    std::string userId = m_client.getCurrentUserId();
    if (userId.empty())
    {
        WEIBO_LOG_ERROR("cannot resolve the current user id, the session may be invalid");
        return {};
    }

    weiboapi::UserInfoResponse weiboUserInfo = m_client.getUserInfo(userId);
    if (weiboUserInfo.data.user.id == 0)
    {
        WEIBO_LOG_ERROR("cannot resolve user info for id: {}", userId);
        return {};
    }

    UserInfo userInfo;
    userInfo.id = std::to_string(weiboUserInfo.data.user.id);
    userInfo.uname = weiboUserInfo.data.user.screen_name;
    if (!weiboUserInfo.data.user.avatar_hd.empty())
    {
        userInfo.facePath = dir + "/" + userInfo.id + ".jpg";
        m_client.downloadImage(weiboUserInfo.data.user.avatar_hd, userInfo.facePath);
        userInfo.facePath = util::localeToUtf8(userInfo.facePath);
    }
    userInfo.vipType = weiboUserInfo.data.user.verified_reason;
    userInfo.home = weiboapi::weiboHomeUrl;
    return userInfo;
}

bool WeiboLogin::supportsLogin() const
{
    return true;
}

std::string WeiboLogin::cookies() const
{
    return m_client.cookies();
}

void WeiboLogin::setCookies(std::string cookies)
{
    m_client.setCookies(cookies);
}

bool WeiboLogin::refreshCookies(std::string cookies)
{
    m_client.setCookies(std::move(cookies));
    return m_client.isLogined();
}

bool WeiboLogin::isLoggedIn() const
{
    return weiboapi::WeiboClient::globalClient().isLogined();
}

bool WeiboLogin::logout()
{
    return m_client.getLogout();
}

std::string WeiboLogin::domain() const
{
    return weiboapi::domain;
}

std::vector<adapter::BaseVideoView> WeiboLogin::history()
{
    return {};
}

const WeiboLogin::LoginResource& WeiboLogin::allResources() const
{
    return m_weiboRes;
}

const std::vector<uint8_t>& WeiboLogin::resource(ResourceIndex index) const
{
    if (index >= 0 && static_cast<std::size_t>(index) < m_weiboRes.size())
    {
        return m_weiboRes.at(index);
    }

    return m_emptyString;
}

int WeiboLogin::pluginId() const
{
    return weiboplugin::pluginID;
}
