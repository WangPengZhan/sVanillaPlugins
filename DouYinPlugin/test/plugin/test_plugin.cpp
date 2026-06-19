#include <array>

#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"

namespace
{
constexpr std::array<const char*, 10> kSupportedUrls = {
    "https://www.douyin.com/video/7000000000000000001",
    "https://www.douyin.com/note/7000000000000000002",
    "https://www.douyin.com/slides/7000000000000000003",
    "https://www.iesdouyin.com/share/video/7000000000000000004/?region=CN",
    "https://www.iesdouyin.com/share/note/7000000000000000005/?region=CN",
    "https://www.iesdouyin.com/share/slides/7000000000000000006/?region=CN",
    "https://www.douyin.com/discover?modal_id=7000000000000000007",
    "https://www.douyin.com/collection/7000000000000000008",
    "https://www.iesdouyin.com/share/mix/detail/7000000000000000009/?region=CN",
    "https://www.iesdouyin.com/share/user/MS4wLjABAAAAUserId?sec_uid=MS4wLjABAAAAUserId",
};
}

TEST(DouYinPluginArtifactTest, PluginMetadataIsAvailableFromSharedLibrary)
{
    initDir("./");
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);

    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);
    const auto& info = plugin->pluginMessage();

    EXPECT_EQ(info.name, "DouYinPlugin");
    EXPECT_EQ(info.pluginId, 8);
    EXPECT_EQ(info.domain, "https://www.douyin.com");

    pluginDeinit();
}

TEST(DouYinPluginArtifactTest, AcceptsSupportedUrlsFromSharedLibrary)
{
    initDir("./");
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);

    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);
    for (const auto* url : kSupportedUrls)
    {
        EXPECT_TRUE(plugin->canParseUrl(url)) << url;
    }

    pluginDeinit();
}
