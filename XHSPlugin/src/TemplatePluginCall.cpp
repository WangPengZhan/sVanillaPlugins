#include "Plugin/XHSPlugin.h"
#include "XHSApi/XHSLog.h"

#include <iostream>

#include <TemplatePluginCall.h>
#include <LoggerRegisterHelpper.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

xhsplugin::XHSPlugin* pPlugin = nullptr;

void initDir(const char* dir)
{
    xhsplugin::XHSPlugin::setDir(dir);
    try
    {
        if (!spdlog::get(xhsModuleName))
        {
            LoggerRegisterHelpper::registerLogger(xhsModuleName, std::string(dir) + "log/" + xhsModuleName + ".log");
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
        pPlugin = new xhsplugin::XHSPlugin;
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
    LoggerRegisterHelpper::unregisterLogger(xhsModuleName);
}
