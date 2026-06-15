#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"

TEST(XHSPluginArtifactTest, PluginMetadataIsAvailableFromSharedLibrary)
{
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);

    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);
    const auto& info = plugin->pluginMessage();

    EXPECT_EQ(info.name, "XHSPlugin");
    EXPECT_EQ(info.pluginId, 7);

    pluginDeinit();
}
