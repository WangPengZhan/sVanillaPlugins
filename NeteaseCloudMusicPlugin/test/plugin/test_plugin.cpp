#include <array>

#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"

namespace
{
constexpr std::array<const char*, 10> kSupportedUrls = {
    "https://music.163.com/song?id=347230",   "https://music.163.com/#/song?id=347231", "https://music.163.com/m/song?id=347232",
    "https://y.music.163.com/song?id=347233", "https://music.163.com/album?id=28521",   "https://music.163.com/#/album?id=28522",
    "https://music.163.com/artist?id=6452",   "https://music.163.com/#/artist?id=6453", "https://music.163.com/playlist?id=3778678",
    "https://music.163.com/mv?id=5436712",
};
}

TEST(NeteaseCloudMusicPluginArtifactTest, PluginMetadataIsAvailableFromSharedLibrary)
{
    initDir("./");
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);

    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);
    const auto& info = plugin->pluginMessage();

    EXPECT_EQ(info.name, "NeteaseCloudMusic");
    EXPECT_EQ(info.pluginId, 5);
    EXPECT_EQ(info.domain, "https://music.163.com/");

    pluginDeinit();
}

TEST(NeteaseCloudMusicPluginArtifactTest, AcceptsSupportedUrlsFromSharedLibrary)
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
