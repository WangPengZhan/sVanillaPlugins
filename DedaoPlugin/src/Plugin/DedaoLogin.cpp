#include "DedaoLogin.h"

#include <BaseVideoView.h>
#include <fstream>

#include "DedaoResource.h"
#include "DedaoApi/DedaoClient.h"
#include "DedaoApi/DedaoApiConstants.h"
#include "PluginCrypto/Encoding.h"
#include "Util/TimerUtil.h"
#include "Util/LocaleHelper.h"
#include "DedaoPlugin.h"
#include "DedaoPluginMessage.h"

DedaoLogin::LoginResource DedaoLogin::m_dedaoRes{qrc_background, qrc_loading, qrc_tip, qrc_waitConfirm, qrc_complete, qrc_init, qrc_refresh};

DedaoLogin::DedaoLogin()
    : AbstractLoginApi()
    , m_client(dedaoapi::DedaoClient::globalClient())
{
}

DedaoLogin::LoginStatus DedaoLogin::getLoginStatus()
{
    std::string token;
    std::string qrString;
    {
        std::lock_guard lc(m_mutexData);
        token = m_token;
        qrString = m_qrString;
    }

    const auto loginStatus = m_client.loginCheck(token, qrString);
    if (loginStatus.data.status == 0)
    {
        return NoScan;
    }
    else if (loginStatus.data.status == 1)
    {
        return Success;
    }
    else if (loginStatus.data.status == 2)
    {
        return Timeout;
    }
    else
    {
        return Error;
    }
}

bool DedaoLogin::getScanContext(std::string& content)
{
    auto token = m_client.loginToken();
    auto res = m_client.loginQrcode(token);
    {
        std::lock_guard lc(m_mutexData);
        m_token = token;
        m_qrString = res.data.qrCodeString;
    }

    std::string path = DedaoPlugin::getDir() + "login/dedao_qrc.png";
    if (!std::filesystem::exists(std::filesystem::path(path).parent_path()))
    {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }

    auto pos = res.data.qrCode.find(",");
    std::string base_content = res.data.qrCode.substr(pos + 1);
    auto bin = encoding::base64Decode(base_content);
    std::ofstream qrFile(path, std::ios::binary);
    qrFile << bin;

    {
        m_token = token;
        m_qrString = res.data.qrCodeString;
    }
    content = util::localeToUtf8(path);

    return true;
}

void DedaoLogin::loginSuccess()
{
    auto v2Token = m_client.getV2Token();
    {
        std::lock_guard lc(m_mutexData);
        m_v2Token = v2Token.c.token;
    }
}

UserInfo DedaoLogin::getUserInfo(std::string dir)
{
    UserInfo userInfo;
    auto dedaoUser = m_client.userInfo();
    userInfo.uname = dedaoUser.c.nickname;
    userInfo.vipType = dedaoUser.c.vip_user.info;
    userInfo.home = dedaoapi::dedaoHomeUrl;
    userInfo.id = dedaoUser.c.uid_hazy;

    if (!dedaoUser.c.avatar.empty())
    {
        std::string path = dir + "/" + dedaoUser.c.uid_hazy + ".png";
        FILE* file = fopen(path.c_str(), "wb");
        m_client.get(dedaoUser.c.avatar, file, network::CurlHeader(), false);
        fclose(file);
        userInfo.facePath = util::localeToUtf8(path);
    }

    return userInfo;
}

bool DedaoLogin::supportsLogin() const
{
    return true;
}

std::string DedaoLogin::cookies() const
{
    return m_client.cookies();
}

void DedaoLogin::setCookies(std::string cookies)
{
    m_client.setCookies(cookies);
}

bool DedaoLogin::refreshCookies(std::string cookies)
{
    return false;
}

bool DedaoLogin::isLoggedIn() const
{
    return m_client.isLogined();
}

bool DedaoLogin::logout()
{
    std::string v2Token;
    {
        std::lock_guard lc(m_mutexData);
        v2Token = m_v2Token;
    }

    return m_client.logout(v2Token);
}

std::string DedaoLogin::domain() const
{
    return dedaoapi::domain;
}

std::vector<adapter::BaseVideoView> DedaoLogin::history()
{
    return {};
}

const DedaoLogin::LoginResource& DedaoLogin::allResources() const
{
    return m_dedaoRes;
}

const std::vector<uint8_t>& DedaoLogin::resource(ResourceIndex index) const
{
    if (index >= 0 && index < m_dedaoRes.size())
    {
        return m_dedaoRes.at(index);
    }

    return m_emptyString;
}

int DedaoLogin::pluginId() const
{
    return dedaoplugin::pluginID;
}
