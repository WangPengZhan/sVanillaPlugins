#include "DouYinPlugin.h"

#include "DouYinApi/DouYinApi.h"
#include "DouYinApi/DouYinClient.h"
#include "DouYinApi/DouYinUrl.h"
#include "DouYinResource.h"
#include "DouYinDownloader.h"
#include "Convert.h"
#include "DouYinPluginMessage.h"

namespace douyinplugin
{

PluginMessage DouYinPlugin::m_pluginMessage = {
    douyinplugin::pluginID, douyinplugin::name, douyinplugin::version, douyinplugin::description, douyinplugin::domain,
};

std::string DouYinPlugin::m_dir;

DouYinPlugin::DouYinPlugin()
    : IPlugin()
    , m_client(douyinapi::DouYinClient::globalClient())
{
}

const PluginMessage& DouYinPlugin::pluginMessage() const
{
    return m_pluginMessage;
}

const std::vector<uint8_t>& DouYinPlugin::websiteIcon()
{
    return qrc_website_icon;
}

bool DouYinPlugin::canParseUrl(const std::string& url)
{
    return douyinapi::isValidUrl(url);
}

adapter::VideoView DouYinPlugin::getVideoView(const std::string& url)
{
    adapter::VideoView views;

    douyinapi::IDInfo idInfo = douyinapi::getID(url);
    douyinapi::IDType idType = idInfo.type;
    std::string id = idInfo.id;
    if (!idInfo.parentId.empty())
    {
        id = idInfo.parentId;
        idType = idInfo.parentIdType;
    }

    switch (idType)
    {
    case douyinapi::IDType::AwemeId:
    {
        auto detail = m_client.getAwemeDetail(id);
        views.push_back(convertAwemeDetail(detail.aweme_detail));
        break;
    }
    case douyinapi::IDType::SeriesId:
    {
        int cursor = 0;
        douyinapi::SeriesDetail detail;
        do
        {
            detail = m_client.getSeriesDetail(id, cursor, 10);
            auto batch = convertSeriesDetail(detail);
            views.insert(views.end(), batch.begin(), batch.end());
            cursor = detail.max_cursor;
        } while (detail.has_more);
        break;
    }
    case douyinapi::IDType::MixId:
    {
        int cursor = 0;
        douyinapi::SeriesDetail detail;
        do
        {
            detail = m_client.getMixDetail(id, cursor, 10);
            auto batch = convertSeriesDetail(detail);
            views.insert(views.end(), batch.begin(), batch.end());
            cursor = detail.cursor;
        } while (detail.has_more);
        break;
    }
    case douyinapi::IDType::UserId:
    {
        break;
    }
    default:
        break;
    }

    return views;
}

std::shared_ptr<download::FileDownloader> DouYinPlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    auto copyedVideoInfo = videoInfo;
    copyedVideoInfo.downloadConfig = std::make_shared<DownloadConfig>(*videoInfo.downloadConfig);
    copyedVideoInfo.videoView = std::make_shared<adapter::BaseVideoView>(*videoInfo.videoView);

    auto detail = m_client.getAwemeDetail(copyedVideoInfo.videoView->Identifier);

    download::ResourceInfo info;
    info.videoUris.push_back(getPlayUrl(detail.aweme_detail.video));
    auto fileName = videoInfo.fileName(true);
    info.option.out = fileName;
    info.option.dir = videoInfo.downloadConfig->downloadDir;
    const std::list<std::string> h = {"Referer: https://www.douyin.com/", std::string("User-Agent: ") + network::chrome, "Origin: https://www.douyin.com",
                                      "Accept: */*"};
    info.option.header = h;

    auto douyinDownlaoder = std::shared_ptr<download::DouYinDownloader>(new download::DouYinDownloader(info), download::freeDownloader);
    return douyinDownlaoder;
}

LoginProxy DouYinPlugin::loginer()
{
    // 暂时不实现登录功能
    return LoginProxy(m_login);
}

void DouYinPlugin::setDir(std::string dir)
{
    m_dir = dir;
}

const std::string& DouYinPlugin::getDir()
{
    return m_dir;
}

}  // namespace douyinplugin
