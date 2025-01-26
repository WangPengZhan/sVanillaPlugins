#include "Plugin/YoutubePlugin.h"

#include <TemplatePluginCall.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

YoutubePlugin* pPlugin = nullptr;

PluginHandle pluginInit()
{
    if (!pPlugin)
    {
        pPlugin = new YoutubePlugin;
    }
    return PluginHandle(pPlugin);
}

void pluginDeinit()
{
    if (pPlugin)
    {
        delete pPlugin;
        pPlugin = nullptr;
    }
}
