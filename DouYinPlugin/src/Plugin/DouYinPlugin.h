#pragma once
#include <string>
#include <vector>
#include <memory>

#include <IPlugin.h>

#include "DouYinApi/DouYinClient.h"
#include "DouYinLogin.h"

namespace douyinplugin
{

class DouYinPlugin : public plugin::IPlugin
{
public:
    DouYinPlugin();
    ~DouYinPlugin() = default;

    const PluginMessage& pluginMessage() const override;
    const std::vector<uint8_t>& websiteIcon() override;
    bool canParseUrl(const std::string& url) override;
    adapter::VideoView getVideoView(const std::string& url) override;
    std::shared_ptr<download::FileDownloader> getDownloader(const VideoInfoFull& videoInfo) override;
    LoginProxy loginer() override;

    static void setDir(std::string dir);
    static const std::string& getDir();

private:
    static PluginMessage m_pluginMessage;
    static std::string m_dir;

    douyinapi::DouYinClient& m_client;
    DouYinLogin m_login;
};

}  // namespace douyinplugin
