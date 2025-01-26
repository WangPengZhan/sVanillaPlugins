#pragma once

#include <LoginWeb.h>

class YoutubeLogin : public AbstractLoginWeb
{
public:
    // thread-safe

    std::string loginUrl() const override;
    void setCookies(std::string cookies) const override;
    std::unordered_set<std::string> mustKeys() const override;

    UserInfo getUserInfo(std::string dir) override;
    bool isLogin() const override;
    bool logout() override;
    std::vector<adapter::BaseVideoView> history() override;

    int type() const override;
};
