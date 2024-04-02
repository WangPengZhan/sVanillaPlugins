#pragma once
#include <PluginConfig.h>

#ifdef __cplusplus
namespace Plugin
{
    class IPlugin;
}

PLUGIN_PORT Plugin::IPlugin *PLUGIN_CALL plugin();

extern "C"
{
#endif
    PLUGIN_PORT const char *PLUGIN_CALL pluginName();
    PLUGIN_PORT const char *PLUGIN_CALL pluginVersion();
#ifdef __cplusplus
}
#endif