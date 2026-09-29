#include "Plugin/YoutubePlugin.h"
#include "YoutubeApi/YoutubeLog.h"

#include <iostream>

#include <TemplatePluginCall.h>
#include <LoggerRegisterHelpper.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

YoutubePlugin* pPlugin = nullptr;

void initDir(const char* dir)
{
    YoutubePlugin::setDir(dir);
    try
    {
        if (!spdlog::get(youtubeModuleName))
        {
            LoggerRegisterHelpper::registerLogger(youtubeModuleName, std::string(dir) + "log/" + youtubeModuleName + ".log");
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

void deinit()
{
    LoggerRegisterHelpper::unregisterLogger(youtubeModuleName);
}
