#include "Plugin/BiliBiliPlugin.h"
#include "BiliApi/BilibiliLog.h"
#include "BiliApi/BilibiliUtils.h"

#include <iostream>

#include <TemplatePluginCall.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

BiliBiliPlugin* pPlugin = nullptr;

namespace
{
constexpr int logFileMaxSize = 100 * 1024 * 1024;  // 20M

void registerLogger(const std::string& logName)
{
    if (!spdlog::get(logName))
    {
        spdlog::rotating_logger_mt<spdlog::async_factory>(logName, "log/" + logName + ".log", logFileMaxSize, 100);
        // spdlog::create_async<spdlog::sinks::basic_file_sink_mt>(logName, "log/" + logName + ".log", logFileMaxSize, 100);
    }
}
}  // namespace

void initLog()
{
    spdlog::init_thread_pool(32768, 1);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%g:%#,%!][%l] %v");
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::trace);
#else
    spdlog::set_level(spdlog::level::info);
#endif
    spdlog::flush_on(spdlog::level::err);
    spdlog::flush_every(std::chrono::seconds(5));

    registerLogger("Network");
    registerLogger("FFmpeg");
    registerLogger("BiliBili");
}

void initDir(const char* dir)
{
    BiliBiliPlugin::setDir(dir);
    biliapi::setCookieDataDir(dir);
    try
    {
        if (!spdlog::get(biliModuleName))
        {
            spdlog::rotating_logger_mt<spdlog::async_factory>(biliModuleName, std::string(dir) + "log/" + biliModuleName + ".log", logFileMaxSize, 100);
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
