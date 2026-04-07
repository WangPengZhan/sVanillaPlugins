#include "XHSLogin.h"

#include <BaseVideoView.h>
#include <fstream>

#include "XHSResource.h"
#include "XHSApi/XHSClient.h"
#include "XHSApi/XHSApiConstants.h"
#include "Util/TimerUtil.h"
#include "Util/LocaleHelper.h"
#include "XHSPlugin.h"
#include "XHSPluginMessage.h"
#include "QrCodeGenerator.h"

XHSLogin::LoginResource XHSLogin::m_xhsRes{qrc_background, qrc_loading, qrc_tip, qrc_waitConfirm, qrc_complete, qrc_init, qrc_refresh};

XHSLogin::XHSLogin()
    : AbstractLoginApi()
    , m_client(xhsapi::XHSClient::globalClient())
{
}

XHSLogin::LoginStatus XHSLogin::getLoginStatus()
{
    std::string code;
    std::string qrId;
    {
        std::lock_guard lc(m_mutexData);
        code = m_code;
        qrId = m_qrId;
    }

    const auto loginStatus = m_client.getLoginStatus(code, qrId);
    if (loginStatus.code != 0)
    {
        return LoginStatus::Unknow;
    }

    if (loginStatus.data.codeStatus == 0)
    {
        return LoginStatus::NoScan;
    }
    else if (loginStatus.data.codeStatus == 1)
    {
        return LoginStatus::ScannedNoAck;
    }
    else if (loginStatus.data.codeStatus == 2)
    {
        return LoginStatus::Success;
    }

    return LoginStatus::Unknow;
}

bool XHSLogin::getScanContext(std::string& content)
{
    const auto login = m_client.getQRCode();
    if (login.data.qr_id.empty() || login.code != 0)
    {
        return false;
    }

    {
        std::lock_guard lc(m_mutexData);
        m_qrId = login.data.qr_id;
        m_code = login.data.code;
    }

    std::string path = xhsplugin::XHSPlugin::getDir() + "login/xhs_qrc.svg";
    if (!std::filesystem::exists(std::filesystem::path(path).parent_path()))
    {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }

    std::ofstream file(path);
    file << QrCodeGenerator::generateQR(login.data.url);
    file.close();

    content = util::localeToUtf8(path);
    return true;
}

void XHSLogin::loginSuccess()
{
}

UserInfo XHSLogin::getUserInfo(std::string dir)
{
    UserInfo userInfo;

    return userInfo;
}

bool XHSLogin::supportsLogin() const
{
    return true;
}

std::string XHSLogin::cookies() const
{
    return m_client.cookies();
}

void XHSLogin::setCookies(std::string cookies)
{
    m_client.setCookies(cookies);
}

bool XHSLogin::refreshCookies(std::string cookies)
{
    return false;
}

bool XHSLogin::isLoggedIn() const
{
    return m_client.isLogined();
}

bool XHSLogin::logout()
{
    return true;
}

std::string XHSLogin::domain() const
{
    return xhsapi::domain;
}

std::vector<adapter::BaseVideoView> XHSLogin::history()
{
    return {};
}

const XHSLogin::LoginResource& XHSLogin::allResources() const
{
    return m_xhsRes;
}

const std::vector<uint8_t>& XHSLogin::resource(ResourceIndex index) const
{
    if (index >= 0 && index < m_xhsRes.size())
    {
        return m_xhsRes.at(index);
    }

    return m_emptyString;
}

int XHSLogin::pluginId() const
{
    return xhsplugin::pluginID;
}
