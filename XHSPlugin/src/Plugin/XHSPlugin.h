#pragma once
#include <string>
#include <vector>
#include <memory>

#include <IPlugin.h>

#include "XHSApi/XHSClient.h"
#include "XHSLogin.h"

namespace xhsplugin
{

class XHSPlugin : public plugin::IPlugin
{
public:
    XHSPlugin();
    ~XHSPlugin() override = default;

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

    xhsapi::XHSClient& m_client;
    XHSLogin m_login;
};

}  // namespace xhsplugin
