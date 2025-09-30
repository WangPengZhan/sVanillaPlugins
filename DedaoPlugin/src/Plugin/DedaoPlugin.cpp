#include "DedaoPlugin.h"
#include "DedaoApi/DedaoClient.h"
#include "DedaoApi/DedaoApi.h"
#include "DedaoApi/DedaoUrl.h"
#include "DedaoApi/DedaoApiConstants.h"
#include "DedaoResource.h"
#include "DedaoDownloader.h"
#include "Convert.h"
#include "DedaoPluginMessage.h"
#include "DedaoApi/DedaoLog.h"
#include "Util/LocaleHelper.h"
#include "Plugin/HLSPlugin.h"

#include <Util/UrlProccess.h>

PluginMessage DedaoPlugin::m_pluginMessage = {
    dedaoplugin::pluginID, dedaoplugin::name, dedaoplugin::version, dedaoplugin::description, dedaoplugin::domain,
};

std::string DedaoPlugin::m_dir;

DedaoPlugin::DedaoPlugin()
    : IPlugin()
    , m_client(dedaoapi::DedaoClient::globalClient())
{
}

const PluginMessage& DedaoPlugin::pluginMessage() const
{
    return m_pluginMessage;
}

const std::vector<uint8_t>& DedaoPlugin::websiteIcon()
{
    return qrc_website_icon;
}

bool DedaoPlugin::canParseUrl(const std::string& url)
{
    return isValidUrl(url);
}

adapter::VideoView DedaoPlugin::getVideoView(const std::string& url)
{
    DEDAO_LOG_INFO("getVideoView url: {}", url);
    auto id = getID(url);
    DEDAO_LOG_INFO("id: {}", id.to_string());
    adapter::VideoView views;

    if (id.type == IDType::Live)
    {
        auto liveInfo = m_client.liveInfo(id.id);
        views = convertVideoView(liveInfo.c);

        for (auto& view : views)
        {
            view.Identifier = id.id;
            view.IdType = typeToString(id.type);
        }
    }
    else if (id.type == IDType::Course)
    {
        auto courseInfo = m_client.courseInfo(id.id);
        int total = courseInfo.c.class_info.current_article_count;
        const int block = 30;
        int request = 0;
        int max_id = 0;
        do
        {
            auto articleInfo = m_client.articleInfo(id.id, block, false, max_id, false, "");
            for (const auto& article : articleInfo.c.article_list)
            {
                auto view = convertVideoView(article);
                view.Publisher = courseInfo.c.class_info.lecturer_name;
                view.IdType = typeToString(IDType::Article);
                views.push_back(std::move(view));
                max_id = article.id;
            }
            request += block;
            auto ms = std::chrono::milliseconds(std::rand() % 1000);
            std::this_thread::sleep_for(ms);
        } while ((total - request) > 0);
    }

    return views;
}

std::shared_ptr<download::FileDownloader> DedaoPlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    auto copyedVideoInfo = videoInfo;
    copyedVideoInfo.downloadConfig = std::make_shared<DownloadConfig>(*videoInfo.downloadConfig);
    copyedVideoInfo.videoView = std::make_shared<adapter::BaseVideoView>(*videoInfo.videoView);
    copyedVideoInfo.videoView->Option1 = std::to_string(1);

    auto type = stringToType(copyedVideoInfo.videoView->IdType);
    if (type == IDType::Live)
    {
        auto liveToken = m_client.livetoken(copyedVideoInfo.videoView->Identifier);
        auto roomDetail = m_client.roomDetail(copyedVideoInfo.videoView->Identifier, liveToken.c.token);
        std::string str_ddmedia = std::to_string(roomDetail.c.playback_info.ddmedia_id);
        auto mediaStreamInfo = m_client.webStreamInfo(copyedVideoInfo.videoView->Identifier, str_ddmedia, roomDetail.c.playback_info.web_pc_media_token);
        std::vector<dedaoapi::Format> formats;
        for (const auto& track : mediaStreamInfo.c.tracks)
        {
            formats.insert(formats.end(), track.formats.begin(), track.formats.end());
        }
        auto it = std::max_element(formats.begin(), formats.end());
        if (it != formats.end())
        {
            HLSPlugin plugin;
            copyedVideoInfo.videoView->Identifier = it->url;
            return plugin.getDownloader(copyedVideoInfo);
        }
    }
    else if (type == IDType::Course)
    {
        auto courseInfo = m_client.courseInfo(copyedVideoInfo.videoView->Option1);
    }
    else if (type == IDType::Article)
    {
        if (!copyedVideoInfo.videoView->Option1.empty())
        {
            HLSPlugin plugin;
            copyedVideoInfo.videoView->Identifier = copyedVideoInfo.videoView->Option1;
            return plugin.getDownloader(copyedVideoInfo);
        }
    }

    return {};
}

LoginProxy DedaoPlugin::loginer()
{
    return LoginProxy(m_login);
}

void DedaoPlugin::setDir(std::string dir)
{
    m_dir = dir;
}

const std::string& DedaoPlugin::getDir()
{
    return m_dir;
}
