#include <array>

#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"

namespace
{
constexpr std::array<const char*, 10> kSupportedUrls = {
    "https://www.xiaohongshu.com/explore/69abf6df0000000028009901?xsec_token=token001",
    "https://www.xiaohongshu.com/discovery/item/69abf6df0000000028009902?xsec_token=token002",
    "https://www.xiaohongshu.com/note/69abf6df0000000028009903?xsec_token=token003",
    "https://xiaohongshu.com/explore/69abf6df0000000028009904?xsec_token=token004",
    "https://www.xiaohongshu.com/explore/69abf6df0000000028009905?app_platform=ios&xsec_token=token005&share_id=abc",
    "https://www.xiaohongshu.com/user/profile/69ac18060000000032019221?xsec_token=token101",
    "https://xiaohongshu.com/user/profile/69ac18060000000032019222?xsec_token=token102",
    "https://www.xiaohongshu.com/user/profile/69ac18060000000032019223?xsec_source=pc_note&xsec_token=token103",
    "https://www.xiaohongshu.com/user/profile/69ac18060000000032019224?app_platform=ios&xsec_token=token104",
    "https://www.xiaohongshu.com/user/profile/69ac18060000000032019225?xsec_token=token105&share_id=abc",
};
}

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

TEST(XHSPluginArtifactTest, AcceptsSupportedUrlsFromSharedLibrary)
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
