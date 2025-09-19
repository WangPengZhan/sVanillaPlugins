#include "BiliBiliLogin.h"

#include <BaseVideoView.h>

#include <fstream>

#include "BiliBiliResource.h"
#include "BiliApi/BilibiliClient.h"
#include "BiliApi/BiliApiConstants.h"
#include "Util/TimerUtil.h"
#include "BiliBiliPlugin.h"
#include "Util/LocaleHelper.h"
#include "QrCodeGenerator.h"
#include "BiliBiliPluginMessage.h"

namespace
{

adapter::BaseVideoView convertHistory(const biliapi::HistoryInfo& data)
{
    auto item = adapter::BaseVideoView();
    item.Identifier = data.history.bvid;
    item.Option1 = std::to_string(data.history.cid);
    item.Option2 = std::to_string(data.history.cid);
    item.Title = data.title;
    item.Cover = data.cover.empty() ? (data.covers.empty() ? "" : data.covers.front()) : data.cover;
    item.Duration = formatDuration(data.duration);
    item.Description = data.new_desc;
    item.PublishDate = convertTimestamp(data.view_at);

    return item;
}

std::vector<adapter::BaseVideoView> convertVideoView(const biliapi::History& data)
{
    std::vector<adapter::BaseVideoView> videoListView;
    videoListView.reserve(256);

    for (const auto& historyInfo : data.data.list)
    {
        videoListView.emplace_back(convertHistory(historyInfo));
    }

    return videoListView;
}
}  // namespace

BiliBiliLogin::LoginResource BiliBiliLogin::m_biliRes{qrc_background, qrc_loading, qrc_tip, qrc_waitConfirm, qrc_complete, qrc_init, qrc_refresh};

BiliBiliLogin::LoginStatus BiliBiliLogin::getLoginStatus()
{
    const auto loginStatus = biliapi::BilibiliClient::globalClient().getLoginStatus(m_qrcodeKey);

    if (loginStatus.code == 0)
    {
        if (loginStatus.data.code == 86038)
        {
            return Timeout;
        }
        else if (loginStatus.data.code == 86101)
        {
            return NoScan;
        }
        else if (loginStatus.data.code == 86090)
        {
            return ScannedNoAck;
        }
        else if (loginStatus.data.code == 0)
        {
            return Success;
        }
        else
        {
            return Error;
        }
    }
    else
    {
        return Error;
    }
}

bool BiliBiliLogin::getScanContext(std::string& content)
{
    const auto login = biliapi::BilibiliClient::globalClient().getLoginUrl();
    if (login.data.qrcode_key.empty() || login.code != 0)
    {
        return false;
    }

    m_qrcodeKey = login.data.qrcode_key;

    std::string path = BiliBiliPlugin::getDir() + "login/bilibili_qrc.svg";
    if (!std::filesystem::exists(std::filesystem::path(path).parent_path()))
    {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }

    std::ofstream file(path);
    file << QrCodeGenerator::generateQR(login.data.url);
    file.close();

    content = util::localeToUtf8(path);
    return true;
}

void BiliBiliLogin::loginSuccess()
{
}

UserInfo BiliBiliLogin::getUserInfo(std::string dir)
{
    UserInfo userInfo;

    const auto nav = biliapi::BilibiliClient::globalClient().getNavInfo();
    if (nav.code != 0)
    {
        return userInfo;
    }
    userInfo.uname = nav.data.uname;

    if (!nav.data.face.empty())
    {
        std::string path = dir + "/" + std::to_string(nav.data.mid) + ".jpg";
        FILE* file = fopen(path.c_str(), "wb");
        network::NetWork netWork;
        biliapi::BilibiliClient::globalClient().get(nav.data.face, file);
        fclose(file);
        userInfo.facePath = path;
    }

    return userInfo;
}

bool BiliBiliLogin::supportsLogin() const
{
    return true;
}

std::string BiliBiliLogin::cookies() const
{
    return biliapi::BilibiliClient::globalClient().cookies();
}

void BiliBiliLogin::setCookies(std::string cookies)
{
    biliapi::BilibiliClient::globalClient().setCookies(cookies);
}

bool BiliBiliLogin::refreshCookies(std::string cookies)
{
    return false;
}

bool BiliBiliLogin::isLoggedIn() const
{
    return biliapi::BilibiliClient::globalClient().isLogined();
}

bool BiliBiliLogin::logout()
{
    const auto logoutInfo = biliapi::BilibiliClient::globalClient().getLogoutExitV2();
    return logoutInfo.code == 0;
}

std::string BiliBiliLogin::domain() const
{
    return biliapi::domain;
}

std::vector<adapter::BaseVideoView> BiliBiliLogin::history()
{
    biliapi::HistoryQueryParam param;
    param.ps = 30;
    const auto historyInfo = biliapi::BilibiliClient::globalClient().getHistory(param);

    return convertVideoView(historyInfo);
}

const BiliBiliLogin::LoginResource& BiliBiliLogin::allResources() const
{
    return m_biliRes;
}

const std::vector<uint8_t>& BiliBiliLogin::resource(ResourceIndex index) const
{
    if (index >= 0 && index < m_biliRes.size())
    {
        return m_biliRes.at(index);
    }

    return m_emptyString;
}

int BiliBiliLogin::pluginId() const
{
    return biliplugin::pluginID;
}
