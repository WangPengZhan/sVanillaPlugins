#include <array>

#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"

namespace
{
constexpr std::array<const char*, 10> kSupportedUrls = {
    "https://www.dedao.cn/live/detail?id=abcdefghijklmnopqrst", "https://dedao.cn/live/detail?id=ABCDEFGHIJKLMNOPQRST1234",
    "https://www.dedao.cn/ebook/reader?id=ebook10001",          "https://dedao.cn/ebook/reader?id=EBOOK20002",
    "https://www.dedao.cn/course/detail?id=course30003",        "https://dedao.cn/course/detail?id=COURSE40004",
    "https://www.dedao.cn/course/article?id=article50005",      "https://dedao.cn/course/article?id=ARTICLE60006",
    "https://www.dedao.cn/course/detail?id=abc123XYZ789",       "https://www.dedao.cn/course/article?id=abc123XYZ789",
};
}

TEST(DedaoPluginArtifactTest, PluginMetadataIsAvailableFromSharedLibrary)
{
    initDir("./");
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);

    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);
    const auto& info = plugin->pluginMessage();

    EXPECT_EQ(info.name, "Dedao");
    EXPECT_EQ(info.pluginId, 6);
    EXPECT_EQ(info.domain, "https://www.dedao.cn/");

    pluginDeinit();
}

TEST(DedaoPluginArtifactTest, AcceptsSupportedUrlsFromSharedLibrary)
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
