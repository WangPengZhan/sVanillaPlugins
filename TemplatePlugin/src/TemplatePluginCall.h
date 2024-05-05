#pragma once
#include <PluginConfig.h>

typedef const char *PSTR;
struct IPlugin;
typedef IPlugin *PIPlugin;

#ifdef __cplusplus

extern "C"
{
#endif
    PIPlugin PLUGIN_API pluginObject();
    PSTR PLUGIN_API pluginName();
    PSTR PLUGIN_API pluginVersion();
    void PLUGIN_API pluginDeinit();
#ifdef __cplusplus
}
#endif