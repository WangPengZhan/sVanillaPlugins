#include <string>

#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"
#include "XHSApi/XHSUrl.h"

TEST(XHSPlugin_Test, init1)
{
    std::cout << "test";
}

TEST(XHSPlugin_Test, INFO)
{
    auto hanlde = pluginInit();
    auto plugin = reinterpret_cast<plugin::IPlugin*>(hanlde);
    auto info = plugin->pluginMessage();
    EXPECT_EQ(info.name, "XHSPlugin");
    EXPECT_EQ(info.pluginId, 7);
}

TEST(XHSPlugin_Test, URL)
{
    auto id = xhsapi::getID("https://www.xiaohongshu.com/explore/123456?xsec_token=123456");
    EXPECT_EQ(id.id, "123456");
    EXPECT_EQ(id.type, xhsapi::IDType::NoteId);
    EXPECT_EQ(id.xsecToken, "123456");

    id = xhsapi::getID("https://www.xiaohongshu.com/user/profile/123456?xsec_token=123456");
    EXPECT_EQ(id.id, "123456");
    EXPECT_EQ(id.type, xhsapi::IDType::UserId);
    EXPECT_EQ(id.xsecToken, "123456");

    id = xhsapi::getID("https://www.xiaohongshu.com/explore/"
                       "69abf6df0000000028009902?app_platform=ios&app_version=9.23&share_from_user_hidden=true&xsec_source=app_share&type=video&xsec_token="
                       "CB5a0J99SmDc_RHBCRBYrC6-AS7FTG0svYxu7iy37r71E=&author_share=1&xhsshare=CopyLink&shareRedId=ODpIMTlKNDk2NzUyOTgwNjY5OTo2Pjs8&apptime="
                       "1774770406&share_id=f698e7f3c25a4de8a248a9f779cbfe4a");
    EXPECT_EQ(id.id, "69abf6df0000000028009902");
    EXPECT_EQ(id.type, xhsapi::IDType::NoteId);
    EXPECT_EQ(id.xsecToken, "CB5a0J99SmDc_RHBCRBYrC6-AS7FTG0svYxu7iy37r71E=");

    id = xhsapi::getID(
        "https://www.xiaohongshu.com/user/profile/69ac18060000000032019228?xsec_token=ABw8eGE87tv_vrLs6nCt1H9f-anWwAkFYRNlZQ7WmwiGo=&xsec_source=pc_note");
    EXPECT_EQ(id.id, "69ac18060000000032019228");
    EXPECT_EQ(id.type, xhsapi::IDType::UserId);
    EXPECT_EQ(id.xsecToken, "ABw8eGE87tv_vrLs6nCt1H9f-anWwAkFYRNlZQ7WmwiGo=");
}
