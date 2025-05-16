#include "Plugin/WeiboPlugin.h"
#include "WeiboApi/WeiboLog.h"
#include "WeiboApi/WeiboUtils.h"

#include <iostream>

#include <TemplatePluginCall.h>
#include <LoggerRegisterHelpper.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

WeiboPlugin* pPlugin = nullptr;

void initDir(const char* dir)
{
    WeiboPlugin::setDir(dir);
    try
    {
        if (!spdlog::get(weiboModuleName))
        {
            LoggerRegisterHelpper::registerLogger(weiboModuleName, std::string(dir) + "log/" + weiboModuleName + ".log");
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

PluginHandle pluginInit()
{
    // initLog();
    if (!pPlugin)
    {
        pPlugin = new WeiboPlugin;
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
    LoggerRegisterHelpper::unregisterLogger(weiboModuleName);
}
