#include <array>

#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"

namespace
{
constexpr std::array<const char*, 10> kSupportedUrls = {
    "https://www.youtube.com/watch?v=abcdefghijk",
    "https://music.youtube.com/watch?v=bcdefghijkl",
    "https://www.youtubekids.com/watch?v=cdefghijklm",
    "https://youtu.be/defghijklmn",
    "https://www.youtube.com/embed/efghijklmno",
    "https://www.youtube.com/shorts/fghijklmnop",
    "https://www.youtube.com/playlist?list=PL0123456789ABCDEF",
    "https://www.youtube.com/channel/UC0123456789ABCDE",
    "https://www.youtube.com/@Sample_Channel",
    "https://www.youtube.com/user/SampleUser",
};
}

TEST(YoutubePluginArtifactTest, PluginMetadataIsAvailableFromSharedLibrary)
{
    initDir("./");
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);

    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);
    const auto& info = plugin->pluginMessage();

    EXPECT_EQ(info.name, "Youtube");
    EXPECT_EQ(info.pluginId, 2);
    EXPECT_EQ(info.domain, "https://www.youtube.com");

    pluginDeinit();
}

TEST(YoutubePluginArtifactTest, AcceptsSupportedUrlsFromSharedLibrary)
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
