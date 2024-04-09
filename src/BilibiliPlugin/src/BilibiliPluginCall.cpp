
#include "BilibiliPluginCall.h"
#include "BilibiliPlugin.h"

BilibiliPlugin *pPlugin = new BilibiliPlugin;

PIPlugin plugin()
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

void  pluginDeinit()
{    
    if (pPlugin)
    {
        delete pPlugin;
        pPlugin = nullptr;
    }
}
