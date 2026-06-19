#include <array>

#include <gtest/gtest.h>

#include "XHSApi/XHSUrl.h"

namespace
{
struct UrlCase
{
    const char* url;
    const char* id;
    const char* token;
    xhsapi::IDType type;
};

constexpr std::array<UrlCase, 10> kUrlCases = {
    {
     {"https://www.xiaohongshu.com/explore/69abf6df0000000028009901?xsec_token=token001", "69abf6df0000000028009901", "token001", xhsapi::IDType::NoteId},
     {"https://www.xiaohongshu.com/discovery/item/69abf6df0000000028009902?xsec_token=token002", "69abf6df0000000028009902", "token002",
         xhsapi::IDType::NoteId},
     {"https://www.xiaohongshu.com/note/69abf6df0000000028009903?xsec_token=token003", "69abf6df0000000028009903", "token003", xhsapi::IDType::NoteId},
     {"https://xiaohongshu.com/explore/69abf6df0000000028009904?xsec_token=token004", "69abf6df0000000028009904", "token004", xhsapi::IDType::NoteId},
     {"https://www.xiaohongshu.com/explore/69abf6df0000000028009905?app_platform=ios&xsec_token=token005&share_id=abc", "69abf6df0000000028009905",
         "token005", xhsapi::IDType::NoteId},
     {"https://www.xiaohongshu.com/user/profile/69ac18060000000032019221?xsec_token=token101", "69ac18060000000032019221", "token101",
         xhsapi::IDType::UserId},
     {"https://xiaohongshu.com/user/profile/69ac18060000000032019222?xsec_token=token102", "69ac18060000000032019222", "token102", xhsapi::IDType::UserId},
     {"https://www.xiaohongshu.com/user/profile/69ac18060000000032019223?xsec_source=pc_note&xsec_token=token103", "69ac18060000000032019223", "token103",
         xhsapi::IDType::UserId},
     {"https://www.xiaohongshu.com/user/profile/69ac18060000000032019224?app_platform=ios&xsec_token=token104", "69ac18060000000032019224", "token104",
         xhsapi::IDType::UserId},
     {"https://www.xiaohongshu.com/user/profile/69ac18060000000032019225?xsec_token=token105&share_id=abc", "69ac18060000000032019225", "token105",
         xhsapi::IDType::UserId},
     }
};
}  // namespace

TEST(XHSUrlUnitTest, ExtractsSupportedUrlIdsAndTokens)
{
    for (const auto& testCase : kUrlCases)
    {
        SCOPED_TRACE(testCase.url);
        EXPECT_TRUE(xhsapi::isValidUrl(testCase.url));

        const auto id = xhsapi::getID(testCase.url);
        EXPECT_EQ(id.id, testCase.id);
        EXPECT_EQ(id.type, testCase.type);
        EXPECT_EQ(id.xsecToken, testCase.token);
    }
}

TEST(XHSUrlUnitTest, RejectsUrlsWithoutXsecToken)
{
    EXPECT_FALSE(xhsapi::isValidUrl("https://www.xiaohongshu.com/explore/69abf6df0000000028009901"));
    EXPECT_FALSE(xhsapi::isValidUrl("https://www.xiaohongshu.com/user/profile/69ac18060000000032019221"));

    const auto id = xhsapi::getID("https://www.xiaohongshu.com/explore/69abf6df0000000028009901");
    EXPECT_TRUE(id.id.empty());
    EXPECT_EQ(id.type, xhsapi::IDType::Unkown);
    EXPECT_TRUE(id.xsecToken.empty());
}
