#include <array>

#include <gtest/gtest.h>

#include "DouYinApi/DouYinUrl.h"

namespace
{
struct UrlCase
{
    const char* url;
    const char* id;
    douyinapi::IDType type;
};

constexpr std::array<UrlCase, 10> kUrlCases = {
    {
     {"https://www.douyin.com/video/7000000000000000001", "7000000000000000001", douyinapi::IDType::AwemeId},
     {"https://www.douyin.com/note/7000000000000000002", "7000000000000000002", douyinapi::IDType::AwemeId},
     {"https://www.douyin.com/slides/7000000000000000003", "7000000000000000003", douyinapi::IDType::AwemeId},
     {"https://www.iesdouyin.com/share/video/7000000000000000004/?region=CN", "7000000000000000004", douyinapi::IDType::AwemeId},
     {"https://www.iesdouyin.com/share/note/7000000000000000005/?region=CN", "7000000000000000005", douyinapi::IDType::AwemeId},
     {"https://www.iesdouyin.com/share/slides/7000000000000000006/?region=CN", "7000000000000000006", douyinapi::IDType::AwemeId},
     {"https://www.douyin.com/discover?modal_id=7000000000000000007", "7000000000000000007", douyinapi::IDType::AwemeId},
     {"https://www.douyin.com/collection/7000000000000000008", "7000000000000000008", douyinapi::IDType::MixId},
     {"https://www.iesdouyin.com/share/mix/detail/7000000000000000009/?region=CN", "7000000000000000009", douyinapi::IDType::MixId},
     {"https://www.iesdouyin.com/share/user/MS4wLjABAAAAUserId?sec_uid=MS4wLjABAAAAUserId", "MS4wLjABAAAAUserId", douyinapi::IDType::UserId},
     }
};
}  // namespace

TEST(DouYinUrlUnitTest, ExtractsSupportedUrlIds)
{
    for (const auto& testCase : kUrlCases)
    {
        SCOPED_TRACE(testCase.url);
        EXPECT_TRUE(douyinapi::isValidUrl(testCase.url));

        const auto id = douyinapi::getID(testCase.url);
        EXPECT_EQ(id.id, testCase.id);
        EXPECT_EQ(id.type, testCase.type);
    }
}

TEST(DouYinUrlUnitTest, ExtractsSeriesShareId)
{
    constexpr char url[] = "https://www.iesdouyin.com/share/series/detail/7000000000000000010/?region=CN";

    EXPECT_TRUE(douyinapi::isValidUrl(url));
    const auto id = douyinapi::getID(url);
    EXPECT_EQ(id.id, "7000000000000000010");
    EXPECT_EQ(id.type, douyinapi::IDType::SeriesId);
}

TEST(DouYinUrlUnitTest, RejectsUnsupportedUrls)
{
    EXPECT_FALSE(douyinapi::isValidUrl("https://www.douyin.com/video/123"));
    EXPECT_EQ(douyinapi::getID("https://example.com/video/7000000000000000001").type, douyinapi::IDType::Unkown);
}
