#pragma once
#include <string>

namespace Plugin
{

#define C_EXPORT_PLUGIN 0

#if C_EXPORT_PLUGIN

    typedef const char *(PluginNameFunc)();
    typedef const char *(PluginVersionFunc)();
    typedef int(PluginIDFunc)();
    typedef void(PluginDeinitFunc)();
    typedef const char *(PluginDescriptionFunc)();

    struct IPlugin
    {
        ~IPlugin()
        {
            if (pluginDeinit)
            {
                pluginDeinit();
            }
        }
        PluginNameFunc pluginName = nullptr;
        PluginVersionFunc pluginVersion = nullptr;
        PluginIDFunc pluginID = nullptr;
        PluginDescriptionFunc pluginDescription = nullptr;
        PluginDeinitFunc pluginDeinit = nullptr;
    };

#else
    class IPlugin
    {
    public:
        virtual ~IPlugin() = default;

        virtual const std::string &pluginName() const = 0;
        virtual const std::string &pluginVersion() const = 0;
        virtual int pluginID() const = 0;
        virtual const std::string &pluginDescription() const = 0;
    };
#endif

} // namespace Plugin
