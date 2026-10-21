#include "Plugin/WeiboPlugin.h"
#include "WeiboApi/WeiboLog.h"
#include "WeiboApi/WeiboUtils.h"

#include <iostream>

#include <TemplatePluginCall.h>
#include <LoggerRegisterHelper.h>

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
            LoggerHelper::registerLogger(weiboModuleName, std::string(dir) + "log/" + weiboModuleName + ".log");
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
    LoggerHelper::unregisterLogger(weiboModuleName);
}
