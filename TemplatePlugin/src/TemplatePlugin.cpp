#include "TemplatePlugin.h"

const std::string &TemplatePlugin::pluginName() const
{
    return m_name;
}

const std::string &TemplatePlugin::pluginVersion() const
{
    return m_version;
}