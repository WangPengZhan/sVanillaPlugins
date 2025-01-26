#include "YoutubeLogin.h"

#include <BaseVideoView.h>

#include "YoutubeResource.h"
#include "YoutubeApi/YoutubeClient.h"
#include "YoutubeApi/YoutubeConstants.h"
#include "Util/TimerUtil.h"

std::string YoutubeLogin::loginUrl() const
{
    return std::string("https://accounts.google.com/ServiceLogin?continue=https://www.youtube.com");
}

void YoutubeLogin::setCookies(std::string cookies) const
{
    youtubeapi::YoutubeClient::globalClient().setCookies(cookies);
}

std::unordered_set<std::string> YoutubeLogin::mustKeys() const
{
    return {
        youtubeapi::youtubeMustKey,
    };
}

UserInfo YoutubeLogin::getUserInfo(std::string dir)
{
    UserInfo userInfo;

    return userInfo;
}

bool YoutubeLogin::isLogin() const
{
    return youtubeapi::YoutubeClient::globalClient().isLogined();
}

bool YoutubeLogin::logout()
{
    return youtubeapi::YoutubeClient::globalClient().isLogined();
}

std::vector<adapter::BaseVideoView> YoutubeLogin::history()
{
    return {};
}

int YoutubeLogin::type() const
{
    return 2;
}
