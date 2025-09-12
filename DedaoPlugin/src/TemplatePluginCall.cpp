#include "Plugin/DedaoPlugin.h"
#include "DedaoApi/DedaoLog.h"

#include <iostream>

#include <TemplatePluginCall.h>
#include <LoggerRegisterHelpper.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

DedaoPlugin* pPlugin = nullptr;

void initDir(const char* dir)
{
    DedaoPlugin::setDir(dir);
    try
    {
        if (!spdlog::get(dedaoModuleName))
        {
            LoggerRegisterHelpper::registerLogger(dedaoModuleName, std::string(dir) + "log/" + dedaoModuleName + ".log");
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
        pPlugin = new DedaoPlugin;
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
    LoggerRegisterHelpper::unregisterLogger(dedaoModuleName);
}
