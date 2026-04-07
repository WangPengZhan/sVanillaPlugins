#include "Plugin/DouYinPlugin.h"
#include "DouYinApi/DouYinLog.h"

#include <iostream>

#include <TemplatePluginCall.h>
#include <LoggerRegisterHelpper.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

douyinplugin::DouYinPlugin* pPlugin = nullptr;

void initDir(const char* dir)
{
    douyinplugin::DouYinPlugin::setDir(dir);
    try
    {
        if (!spdlog::get(douyinModuleName))
        {
            LoggerRegisterHelpper::registerLogger(douyinModuleName, std::string(dir) + "log/" + douyinModuleName + ".log");
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
        pPlugin = new douyinplugin::DouYinPlugin;
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
    LoggerRegisterHelpper::unregisterLogger(douyinModuleName);
}
