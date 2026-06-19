#include <array>

#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"

namespace
{
constexpr std::array<const char*, 10> kSupportedUrls = {
    "https://m.weibo.cn/status/N1a2b3c4D",
    "https://m.weibo.cn/detail/N2a2b3c4D",
    "https://weibo.com/1234567890/N3a2b3c4D",
    "https://www.weibo.com/1234567890/N4a2b3c4D",
    "https://weibo.com/0987654321/N5a2b3c4D",
    "https://video.weibo.com/show?fid=1034:abcdef1234567890",
    "https://weibo.com/tv/show/1034:abcdef1234567891",
    "https://h5.video.weibo.com/show/1034:abcdef1234567892",
    "https://video.weibo.com/show?fid=1034:ABCDEF1234567893",
    "https://weibo.com/tv/show/1034:ABCDEF1234567894",
};
}

TEST(WeiboPluginArtifactTest, PluginMetadataIsAvailableFromSharedLibrary)
{
    initDir("./");
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);

    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);
    const auto& info = plugin->pluginMessage();

    EXPECT_EQ(info.name, "WeiBo");
    EXPECT_EQ(info.pluginId, 4);
    EXPECT_EQ(info.domain, "https://weibo.com");

    pluginDeinit();
}

TEST(WeiboPluginArtifactTest, AcceptsSupportedUrlsFromSharedLibrary)
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
