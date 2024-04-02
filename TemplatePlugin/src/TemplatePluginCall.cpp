
#include "TemplatePluginCall.h"
#include "TemplatePlugin.h"

TemplatePlugin *pPlugin = new TemplatePlugin;

Plugin::IPlugin *plugin()
{
    return pPlugin;
}

const char *pluginName()
{
    return pPlugin->pluginName().c_str();
}

const char *pluginVersion()
{
    return pPlugin->pluginVersion().c_str();
}
