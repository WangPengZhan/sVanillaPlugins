#include <gtest/gtest.h>

#include "XHSApi/XHSUrl.h"

TEST(XHSUrlUnitTest, ExtractsNoteIdAndXsecToken)
{
    const auto id = xhsapi::getID("https://www.xiaohongshu.com/explore/123456?xsec_token=123456");

    EXPECT_EQ(id.id, "123456");
    EXPECT_EQ(id.type, xhsapi::IDType::NoteId);
    EXPECT_EQ(id.xsecToken, "123456");
}

TEST(XHSUrlUnitTest, ExtractsUserIdAndXsecToken)
{
    const auto id = xhsapi::getID("https://www.xiaohongshu.com/user/profile/123456?xsec_token=123456");

    EXPECT_EQ(id.id, "123456");
    EXPECT_EQ(id.type, xhsapi::IDType::UserId);
    EXPECT_EQ(id.xsecToken, "123456");
}

TEST(XHSUrlUnitTest, ExtractsSharedNoteTokenFromLongUrl)
{
    const auto id = xhsapi::getID("https://www.xiaohongshu.com/explore/"
                                  "69abf6df0000000028009902?app_platform=ios&app_version=9.23&share_from_user_hidden=true&xsec_source=app_share&type=video&"
                                  "xsec_token=CB5a0J99SmDc_RHBCRBYrC6-AS7FTG0svYxu7iy37r71E=&author_share=1&xhsshare=CopyLink&shareRedId="
                                  "ODpIMTlKNDk2NzUyOTgwNjY5OTo2Pjs8&apptime=1774770406&share_id=f698e7f3c25a4de8a248a9f779cbfe4a");

    EXPECT_EQ(id.id, "69abf6df0000000028009902");
    EXPECT_EQ(id.type, xhsapi::IDType::NoteId);
    EXPECT_EQ(id.xsecToken, "CB5a0J99SmDc_RHBCRBYrC6-AS7FTG0svYxu7iy37r71E=");
}

TEST(XHSUrlUnitTest, ExtractsSharedUserTokenFromLongUrl)
{
    const auto id = xhsapi::getID(
        "https://www.xiaohongshu.com/user/profile/69ac18060000000032019228?xsec_token=ABw8eGE87tv_vrLs6nCt1H9f-anWwAkFYRNlZQ7WmwiGo=&xsec_source=pc_note");

    EXPECT_EQ(id.id, "69ac18060000000032019228");
    EXPECT_EQ(id.type, xhsapi::IDType::UserId);
    EXPECT_EQ(id.xsecToken, "ABw8eGE87tv_vrLs6nCt1H9f-anWwAkFYRNlZQ7WmwiGo=");
}

TEST(XHSUrlUnitTest, RejectsUrlsWithoutXsecToken)
{
    EXPECT_FALSE(xhsapi::isValidUrl("https://www.xiaohongshu.com/explore/123456"));
    EXPECT_FALSE(xhsapi::isValidUrl("https://www.xiaohongshu.com/user/profile/123456"));

    const auto id = xhsapi::getID("https://www.xiaohongshu.com/explore/123456");
    EXPECT_TRUE(id.id.empty());
    EXPECT_EQ(id.type, xhsapi::IDType::Unkown);
    EXPECT_TRUE(id.xsecToken.empty());
}
