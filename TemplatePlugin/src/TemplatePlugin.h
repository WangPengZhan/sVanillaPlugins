#pragma once
#include <IPlugin.h>

class TemplatePlugin : public Plugin::IPlugin
{
public:
    const std::string &pluginName() const override;
    const std::string &pluginVersion() const override;

private:
    std::string m_name{"template"};
    std::string m_version{"1.0.0"};
};