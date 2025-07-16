#pragma once

#include <LoginApi.h>

namespace netease
{
class NeteaseCloudMusicClient;
}

class NeteaseColudMusicLogin : public AbstractLoginApi
{
public:
    NeteaseColudMusicLogin();
    ~NeteaseColudMusicLogin() = default;

    // thread-safe
    LoginSatus getLoginStatus() override;
    bool getScanContext(std::string& content) override;
    void loginSuccess() override;

    bool supportLogin() const override;
    std::string cookies() const override;
    void setCookies(std::string cookies) override;
    bool refreshCookies(std::string cookies) override;
    bool isLogin() const override;
    bool logout() override;

    UserInfo getUserInfo(std::string dir) override;
    std::vector<adapter::BaseVideoView> history() override;

    // resource
    const LoginResource& allResources() const override;
    const std::vector<uint8_t>& resource(ResourceIndex index) const override;

    int type() const override;

private:
    netease::NeteaseCloudMusicClient& m_client;
    std::string m_qrcodeKey;
    static LoginResource m_neteaseRes;
};
