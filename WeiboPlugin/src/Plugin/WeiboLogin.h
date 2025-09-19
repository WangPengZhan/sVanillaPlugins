#pragma once
#include <mutex>

#include <LoginApi.h>

namespace weiboapi
{
class WeiboClient;
}

class WeiboLogin : public AbstractLoginApi
{
public:
    WeiboLogin();
    ~WeiboLogin() = default;
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
    weiboapi::WeiboClient& m_client;
    std::mutex m_mutexData;
    std::string m_qrid;
    std::string m_alt;
    static LoginResource m_weiboRes;
};
