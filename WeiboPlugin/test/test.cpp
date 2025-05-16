#include <string>

#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"

TEST(TemplatePlugin_Test, test)
{
    std::cout << "test";
}

TEST(TemplatePlugin_Test, INFO)
{
    initDir("D:/");
    auto hanlde = pluginInit();
    auto plugin = reinterpret_cast<plugin::IPlugin*>(hanlde);
    auto info = plugin->pluginMessage();
    EXPECT_EQ(info.name, "WeiBo");
    EXPECT_EQ(info.pluginId, 4);
    EXPECT_EQ(info.domain, "https://weibo.com");
}