#include "YoutubeLogin.h"

#include <BaseVideoView.h>

#include "YoutubeResource.h"
#include "YoutubeApi/YoutubeClient.h"
#include "YoutubeApi/YoutubeConstants.h"
#include "Util/TimerUtil.h"
#include "YoutubePluginMessage.h"

bool YoutubeLogin::supportLogin() const
{
    return true;
}

std::string YoutubeLogin::loginUrl() const
{
    return std::string("https://accounts.google.com/ServiceLogin?continue=https://www.youtube.com");
}

void YoutubeLogin::setCookie(std::string cookie)
{
    youtubeapi::YoutubeClient::globalClient().setCookie(cookie);
}

void YoutubeLogin::setCookies(std::string cookies)
{
    youtubeapi::YoutubeClient::globalClient().setCookies(cookies);
}

std::string YoutubeLogin::cookies() const
{
    return youtubeapi::YoutubeClient::globalClient().cookies();
}

bool YoutubeLogin::refreshCookies(std::string cookies)
{
    return false;
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

int YoutubeLogin::pluginId() const
{
    return youtubeplugin::pluginID;
}
