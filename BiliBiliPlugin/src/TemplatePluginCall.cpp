#include "Plugin/BiliBiliPlugin.h"
#include "BiliApi/BilibiliLog.h"
#include "BiliApi/BilibiliUtils.h"

#include <iostream>

#include <TemplatePluginCall.h>
#include <LoggerRegisterHelpper.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

BiliBiliPlugin* pPlugin = nullptr;

void initDir(const char* dir)
{
    BiliBiliPlugin::setDir(dir);
    biliapi::setCookieDataDir(dir);
    try
    {
        if (!spdlog::get(biliModuleName))
        {
            LoggerRegisterHelpper::registerLogger(biliModuleName, std::string(dir) + "log/" + biliModuleName + ".log");
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
        pPlugin = new BiliBiliPlugin;
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
    LoggerRegisterHelpper::unregisterLogger(biliModuleName);
}
