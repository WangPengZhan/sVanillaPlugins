
#include "TemplatePluginCall.h"
#include "TemplatePlugin.h"

TemplatePlugin *pPlugin = new TemplatePlugin;

PIPlugin pluginObject()
{
    return reinterpret_cast<PIPlugin>(pPlugin);
}

const char *pluginName()
{
    if (!pPlugin)
    {
        return "";
    }

    return pPlugin->pluginName().c_str();
}

const char *pluginVersion()
{
    if (!pPlugin)
    {
        return "";
    }

    return pPlugin->pluginVersion().c_str();
}

void pluginDeinit()
{
    if (pPlugin)
    {
        delete pPlugin;
        pPlugin = nullptr;
    }
}
