#include "NeteaseCloudMusicLogin.h"

#include <fstream>

#include <BaseVideoView.h>

#include "NeteaseCloudMusicResource.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicClient.h"
#include "NeteaseCloudMusicPlugin.h"
#include "QrCodeGenerator.h"
#include "Util/LocaleHelper.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicConstants.h"

NeteaseColudMusicLogin::LoginResource NeteaseColudMusicLogin::m_neteaseRes{qrc_background, qrc_loading, qrc_tip,    qrc_waitConfirm,
                                                                           qrc_complete,   qrc_init,    qrc_refresh};

NeteaseColudMusicLogin::NeteaseColudMusicLogin()
    : AbstractLoginApi()
    , m_client(netease::NeteaseCloudMusicClient::globalClient())

{
}

AbstractLoginApi::LoginSatus NeteaseColudMusicLogin::getLoginStatus()
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
        return ScanedNoAck;
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

bool NeteaseColudMusicLogin::getScanContext(std::string& content)
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

void NeteaseColudMusicLogin::loginSuccess()
{
}

bool NeteaseColudMusicLogin::supportLogin() const
{
    return true;
}

std::string NeteaseColudMusicLogin::cookies() const
{
    return {};
}

void NeteaseColudMusicLogin::setCookies(std::string cookies)
{
}
bool NeteaseColudMusicLogin::refreshCookies(std::string cookies)
{
    return false;
}

bool NeteaseColudMusicLogin::isLogin() const
{
    return false;
}

bool NeteaseColudMusicLogin::logout()
{
    return false;
}

UserInfo NeteaseColudMusicLogin::getUserInfo(std::string dir)
{
    return {};
}
std::vector<adapter::BaseVideoView> NeteaseColudMusicLogin::history()
{
    return {};
}
const AbstractLoginApi::LoginResource& NeteaseColudMusicLogin::allResources() const
{
    return m_neteaseRes;
}

const std::vector<uint8_t>& NeteaseColudMusicLogin::resource(ResourceIndex index) const
{
    if (index >= 0 && index < m_neteaseRes.size())
    {
        return m_neteaseRes.at(index);
    }

    return m_emptyString;
}
int NeteaseColudMusicLogin::type() const
{
    return 0;
}
