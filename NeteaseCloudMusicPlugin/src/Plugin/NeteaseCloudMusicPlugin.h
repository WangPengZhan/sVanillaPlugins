#pragma once
#include <IPlugin.h>
#include <memory>

#include "NeteaseCloudMusicLogin.h"

namespace netease
{
class NeteaseCloudMusicClient;
}

class NeteaseCloudMusicPlugin : public plugin::IPlugin
{
public:
    NeteaseCloudMusicPlugin();
    const PluginMessage& pluginMessage() const override;

    const std::vector<uint8_t>& websiteIcon() override;

    bool canParseUrl(const std::string& url) override;
    adapter::VideoView getVideoView(const std::string& url) override;

    std::shared_ptr<download::FileDownloader> getDownloader(const VideoInfoFull& videoInfo) override;

    LoginProxy loginer() override;

    static void setDir(std::string dir);
    static const std::string& getDir();

private:
    static std::string m_dir;
    static PluginMessage m_pluginMessage;
    NeteaseColudMusicLogin m_login;
    netease::NeteaseCloudMusicClient& m_client;
};
