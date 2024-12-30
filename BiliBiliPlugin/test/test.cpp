#include <string>

#include <gtest/gtest.h>

#include "TemplatePluginCall.h"
#include "IPlugin.h"

TEST(TemplatePlugin_Test, test)
{
    std::cout << "test";
}

TEST(TemplatePlugin_Test, INFO)
{
    auto hanlde = pluginInit();
    auto plugin = reinterpret_cast<plugin::IPlugin *>(hanlde);
    auto info = plugin->pluginMessage();
    EXPECT_EQ(info.name, "template");
    EXPECT_EQ(info.pluginId, 0);
    EXPECT_EQ(info.domain, "https://sVanilla.app");
}