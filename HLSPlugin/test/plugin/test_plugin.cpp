#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"

TEST(HLSPluginArtifactTest, PluginMetadataIsAvailableFromSharedLibrary)
{
    initDir("./");
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);

    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);
    const auto& info = plugin->pluginMessage();

    EXPECT_EQ(info.name, "HLSStream");
    EXPECT_EQ(info.pluginId, 3);
    EXPECT_EQ(info.domain, "https://developer.apple.com/streaming/");

    pluginDeinit();
}
