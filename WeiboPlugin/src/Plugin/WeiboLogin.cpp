#include "WeiboLogin.h"

#include <BaseVideoView.h>

#include "WeiboResource.h"
#include "WeiboApi/WeiboClient.h"
#include "Util/TimerUtil.h"
#include "Util/LocaleHelper.h"
#include "WeiboPlugin.h"

WeiboLogin::LoginResource WeiboLogin::m_weiboRes{qrc_background, qrc_loading, qrc_tip, qrc_waitConfirm, qrc_complete, qrc_init, qrc_refresh};

WeiboLogin::WeiboLogin()
    : AbstractLoginApi()
    , m_client(weiboapi::WeiboClient::globalClient())
{
}

WeiboLogin::LoginSatus WeiboLogin::getLoginStatus()
{
    const auto loginStatus = m_client.getLoginStatus(m_qrid);

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
        return ScanedNoAck;
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
    m_qrid = qrc.data.qrid;
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
}

UserInfo WeiboLogin::getUserInfo(std::string dir)
{
    UserInfo userInfo;

    return userInfo;
}

bool WeiboLogin::supportLogin() const
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
    return false;
}

bool WeiboLogin::isLogin() const
{
    return weiboapi::WeiboClient::globalClient().isLogined();
}

bool WeiboLogin::logout()
{
    return false;
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
    if (index >= 0 && index < m_weiboRes.size())
    {
        return m_weiboRes.at(index);
    }

    return m_emptyString;
}

int WeiboLogin::type() const
{
    return 4;
}
