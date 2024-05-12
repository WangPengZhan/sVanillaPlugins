#include "TemplatePlugin.h"

const std::string &TemplatePlugin::pluginName() const
{
    return m_name;
}

const std::string &TemplatePlugin::pluginVersion() const
{
    return m_version;
}

int TemplatePlugin::pluginID() const
{
    return 0;
}

const std::string &TemplatePlugin::pluginDescription() const
{
    return m_description;
}