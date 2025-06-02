#include <iostream>

#include <TemplatePluginCall.h>
#include <LoggerRegisterHelpper.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

#include "Plugin/HLSPlugin.h"
#include "HLSApi/HLSLog.h"

HLSPlugin* pPlugin = nullptr;

void initDir(const char* dir)
{
    HLSPlugin::setDir(dir);
    try
    {
        if (!spdlog::get(HLSModuleName))
        {
            LoggerRegisterHelpper::registerLogger(HLSModuleName, std::string(dir) + "log/" + HLSModuleName + ".log");
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
        pPlugin = new HLSPlugin;
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
    LoggerRegisterHelpper::unregisterLogger(HLSModuleName);
}
