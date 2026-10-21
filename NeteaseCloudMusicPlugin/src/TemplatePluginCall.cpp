#include "Plugin/NeteaseCloudMusicPlugin.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicLog.h"

#include <iostream>

#include <TemplatePluginCall.h>
#include <LoggerRegisterHelper.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

NeteaseCloudMusicPlugin* pPlugin = nullptr;

void initDir(const char* dir)
{
    NeteaseCloudMusicPlugin::setDir(dir);
    try
    {
        if (!spdlog::get(neteaseCloudMusicMuleName))
        {
            LoggerHelper::registerLogger(neteaseCloudMusicMuleName, std::string(dir) + "log/" + neteaseCloudMusicMuleName + ".log");
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
        pPlugin = new NeteaseCloudMusicPlugin;
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
    LoggerHelper::unregisterLogger(neteaseCloudMusicMuleName);
}
