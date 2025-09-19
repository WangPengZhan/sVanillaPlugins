#pragma once

#include <LoginApi.h>

class BiliBiliLogin : public AbstractLoginApi
{
public:
    // thread-safe
    LoginStatus getLoginStatus() override;
    bool getScanContext(std::string& content) override;
    void loginSuccess() override;

    bool supportsLogin() const override;
    std::string cookies() const override;
    void setCookies(std::string cookies) override;
    bool refreshCookies(std::string cookies) override;
    bool isLoggedIn() const override;
    bool logout() override;

    std::string domain() const override;

    UserInfo getUserInfo(std::string dir) override;
    std::vector<adapter::BaseVideoView> history() override;

    // resource
    const LoginResource& allResources() const override;
    const std::vector<uint8_t>& resource(ResourceIndex index) const override;

    int pluginId() const override;

private:
    std::string m_qrcodeKey;
    static LoginResource m_biliRes;
};
