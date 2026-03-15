#include "NeteaseCloudMusicLogin.h"

#include <fstream>

#include <BaseVideoView.h>

#include "NeteaseCloudMusicResource.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicClient.h"
#include "NeteaseCloudMusicPlugin.h"
#include "QrCodeGenerator.h"
#include "Util/LocaleHelper.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicConstants.h"
#include "NeteaseCloudMusicPluginMessage.h"

NeteaseCloudMusicLogin::LoginResource NeteaseCloudMusicLogin::m_neteaseRes{qrc_background, qrc_loading, qrc_tip,    qrc_waitConfirm,
                                                                           qrc_complete,   qrc_init,    qrc_refresh};

NeteaseCloudMusicLogin::NeteaseCloudMusicLogin()
    : AbstractLoginApi()
    , m_client(netease::NeteaseCloudMusicClient::globalClient())

{
}

AbstractLoginApi::LoginStatus NeteaseCloudMusicLogin::getLoginStatus()
{
    auto loginStatus = m_client.getLoginStatus(m_qrcodeKey);
    if (loginStatus.code == 800)
    {
        return Timeout;
    }
    else if (loginStatus.code == 801)
    {
        return NoScan;
    }
    else if (loginStatus.code == 802)
    {
        return ScannedNoAck;
    }
    else if (loginStatus.code == 803)
    {
        return Success;
    }
    else
    {
        return Error;
    }
}

bool NeteaseCloudMusicLogin::getScanContext(std::string& content)
{
    const auto loginKey = m_client.getLoginKey();
    if (loginKey.unikey.empty() || loginKey.code != 200)
    {
        return false;
    }

    m_qrcodeKey = loginKey.unikey;

    std::string path = NeteaseCloudMusicPlugin::getDir() + "login/netease_qrc.svg";
    if (!std::filesystem::exists(std::filesystem::path(path).parent_path()))
    {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }

    const std::string url = netease::Login::QRCreate + m_qrcodeKey;
    std::ofstream file(path);
    file << QrCodeGenerator::generateQR(netease::Login::QRCreate + m_qrcodeKey);
    file.close();

    content = util::localeToUtf8(path);
    return true;
}

void NeteaseCloudMusicLogin::loginSuccess()
{
}

bool NeteaseCloudMusicLogin::supportsLogin() const
{
    return true;
}

std::string NeteaseCloudMusicLogin::cookies() const
{
    return m_client.cookies();
}

void NeteaseCloudMusicLogin::setCookies(std::string cookies)
{
    m_client.setCookies(cookies);
}
bool NeteaseCloudMusicLogin::refreshCookies(std::string cookies)
{
    m_client.setCookies(cookies);
    return false;
}

bool NeteaseCloudMusicLogin::isLoggedIn() const
{
    return m_client.isLogined();
}

bool NeteaseCloudMusicLogin::logout()
{
    return false;
}

std::string NeteaseCloudMusicLogin::domain() const
{
    return netease::domain;
}

UserInfo NeteaseCloudMusicLogin::getUserInfo(std::string dir)
{
    netease::AccountResponse accountInfo = m_client.getAccoutInfo();
    UserInfo userInfo;
    userInfo.id = std::to_string(accountInfo.account.id);
    userInfo.uname = accountInfo.profile.nickname;
    if (!accountInfo.profile.avatarUrl.empty())
    {
        userInfo.facePath = dir + "/" + userInfo.id + ".jpg";
        FILE* file = fopen(userInfo.facePath.c_str(), "wb");
        m_client.get(accountInfo.profile.avatarUrl, file, network::CurlHeader(), false);
        fclose(file);
        userInfo.facePath = util::localeToUtf8(userInfo.facePath);
    }
    userInfo.vipType = std::to_string(accountInfo.account.vipType);
    userInfo.home = netease::home;

    return userInfo;
}

std::vector<adapter::BaseVideoView> NeteaseCloudMusicLogin::history()
{
    return {};
}
const AbstractLoginApi::LoginResource& NeteaseCloudMusicLogin::allResources() const
{
    return m_neteaseRes;
}

const std::vector<uint8_t>& NeteaseCloudMusicLogin::resource(ResourceIndex index) const
{
    if (index >= 0 && index < m_neteaseRes.size())
    {
        return m_neteaseRes.at(index);
    }

    return m_emptyString;
}
int NeteaseCloudMusicLogin::pluginId() const
{
    return neteaseplugin::pluginID;
}
