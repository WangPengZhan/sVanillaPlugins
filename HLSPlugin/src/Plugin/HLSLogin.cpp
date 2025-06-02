#include "HLSLogin.h"

#include <BaseVideoView.h>

#include "HLSResource.h"
#include "HLSApi/HLSClient.h"
#include "Util/TimerUtil.h"

HLSLogin::LoginResource HLSLogin::m_biliRes{qrc_background, qrc_loading, qrc_tip, qrc_waitConfirm, qrc_complete, qrc_init, qrc_refresh};

HLSLogin::LoginSatus HLSLogin::getLoginStatus()
{
    return Error;
}

bool HLSLogin::getScanContext(std::string& content)
{
    return true;
}

void HLSLogin::loginSuccess()
{
}

UserInfo HLSLogin::getUserInfo(std::string dir)
{
    UserInfo userInfo;

    return userInfo;
}

std::string HLSLogin::cookies() const
{
    return {};
}
void HLSLogin::setCookies(std::string cookies)
{
}

bool HLSLogin::refreshCookies(std::string cookies)
{
    return false;
}

bool HLSLogin::isLogin() const
{
    return false;
}

bool HLSLogin::logout()
{
    return false;
}

std::vector<adapter::BaseVideoView> HLSLogin::history()
{
    return {};
}

const HLSLogin::LoginResource& HLSLogin::allResources() const
{
    return m_biliRes;
}

const std::vector<uint8_t>& HLSLogin::resource(ResourceIndex index) const
{
    if (index >= 0 && index < m_biliRes.size())
    {
        return m_biliRes.at(index);
    }

    return m_emptyString;
}

int HLSLogin::type() const
{
    return 3;
}
