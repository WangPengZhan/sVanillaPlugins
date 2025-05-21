#pragma once

#include <LoginWeb.h>

class YoutubeLogin : public AbstractLoginWeb
{
public:
    // thread-safe
    std::string loginUrl() const override;
    void setCookie(std::string cookie) override;
    std::unordered_set<std::string> mustKeys() const override;

    std::string cookies() const override;
    void setCookies(std::string cookies) override;
    bool refreshCookies(std::string cookies) override;
    bool isLogin() const override;
    bool logout() override;

    UserInfo getUserInfo(std::string dir) override;
    std::vector<adapter::BaseVideoView> history() override;

    int type() const override;
};
