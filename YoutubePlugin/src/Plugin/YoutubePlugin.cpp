#include "YoutubePlugin.h"
#include "YoutubeApi/YoutubeClient.h"
#include "YoutubeApi/YoutubeApi.h"
#include "YoutubeApi/YoutubeUrl.h"
#include "YoutubeResource.h"
#include "YoutubeDownloader.h"
#include "Convert.h"
#include "YoutubePluginMessage.h"

#include <Util/UrlProccess.h>

PluginMessage YoutubePlugin::m_pluginMessage = {
    youtubelugin::pluginID, youtubelugin::name, youtubelugin::version, youtubelugin::description, youtubelugin::domain,
};

const PluginMessage& YoutubePlugin::pluginMessage() const
{
    return m_pluginMessage;
}

const std::vector<uint8_t>& YoutubePlugin::websiteIcon()
{
    return qrc_website_icon;
}

bool YoutubePlugin::canParseUrl(const std::string& url)
{
    return isValidUrl(url);
}

adapter::VideoView YoutubePlugin::getVideoView(const std::string& url)
{
    adapter::VideoView views;
    std::string id = getID(url);
    auto& client = youtubeapi::YoutubeClient::globalClient();
    client.getVideoInfo(id);
    client.getBaseJs(client.getIFrameVersion());
    return views;
}

std::shared_ptr<download::FileDownloader> YoutubePlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    return {};
}

LoginProxy YoutubePlugin::loginer()
{
    return LoginProxy(m_login);
}
