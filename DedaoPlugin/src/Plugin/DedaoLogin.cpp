#include "DedaoLogin.h"

#include <BaseVideoView.h>
#include <fstream>

#include "DedaoResource.h"
#include "DedaoApi/DedaoClient.h"
#include "Util/TimerUtil.h"
#include "Util/LocaleHelper.h"
#include "DedaoPlugin.h"
#include "DedaoPluginMessage.h"

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
    auto bin = base64_decode(base_content);
    std::ofstream qrFile(path, std::ios::binary);
    auto strBin = std::string(bin.begin(), bin.end());
    qrFile << strBin;

    {
        m_token = token;
        m_qrString = res.data.qrCodeString;
    }
    content = util::localeToUtf8(path);

    return true;
}

void DedaoLogin::loginSuccess()
{
}

UserInfo DedaoLogin::getUserInfo(std::string dir)
{
    UserInfo userInfo;
    auto dedaoUser = m_client.userInfo();
    userInfo.uname = dedaoUser.c.nickname;
    userInfo.facePath = dedaoUser.c.avatar;
    userInfo.vipType = dedaoUser.c.vip_user.info;
    return userInfo;
}

bool DedaoLogin::supportLogin() const
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

bool DedaoLogin::isLogin() const
{
    return m_client.isLogined();
}

bool DedaoLogin::logout()
{
    return false;
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
