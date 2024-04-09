#pragma once
#include "IPlugin.h"

class YoutubePlugin : public Plugin::IPlugin
{
public:
    [[nodiscard]] const std::string& pluginName() const override;
    [[nodiscard]] const std::string& pluginVersion() const override;

private:
    std::string m_name{"youtube"};
    std::string m_version{"1.0.0"};
};