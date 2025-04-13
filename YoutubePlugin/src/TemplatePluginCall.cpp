#include "Plugin/YoutubePlugin.h"
#include "YoutubeApi/YoutubeLog.h"

#include <iostream>

#include <TemplatePluginCall.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

YoutubePlugin* pPlugin = nullptr;
constexpr int logFileMaxSize = 100 * 1024 * 1024;  // 20M

void initDir(const char* dir)
{
    YoutubePlugin::setDir(dir);
    try
    {
        if (!spdlog::get(youtubeModuleName))
        {
            spdlog::rotating_logger_mt<spdlog::async_factory>(youtubeModuleName, std::string(dir) + "log/" + youtubeModuleName + ".log", logFileMaxSize, 100);
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

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
