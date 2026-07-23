#include <array>

#include <gtest/gtest.h>

#include "DouYinApi/DouYinUrl.h"
#include "DouYinApi/DouYinUtils.h"
#include "PluginCrypto/Crypto.h"
#include "PluginCrypto/Encoding.h"

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

TEST(DouYinUrlUnitTest, ExtractsUserPageAndModalWork)
{
    constexpr char userUrl[] = "https://www.douyin.com/user/MS4wLjABAAAAUserId";
    auto id = douyinapi::getID(userUrl);
    EXPECT_EQ(id.id, "MS4wLjABAAAAUserId");
    EXPECT_EQ(id.type, douyinapi::IDType::UserId);

    constexpr char modalUrl[] = "https://www.douyin.com/user/MS4wLjABAAAAUserId?modal_id=7000000000000000011";
    id = douyinapi::getID(modalUrl);
    EXPECT_EQ(id.id, "MS4wLjABAAAAUserId");
    EXPECT_EQ(id.type, douyinapi::IDType::UserId);
    EXPECT_EQ(id.parentId, "7000000000000000011");
    EXPECT_EQ(id.parentIdType, douyinapi::IDType::AwemeId);
}

TEST(DouYinUrlUnitTest, ExtractsSearchAndChannelModalWork)
{
    for (const char* url :
         {"https://www.douyin.com/search/example?modal_id=7000000000000000012", "https://www.douyin.com/channel/123?modal_id=7000000000000000013"})
    {
        const auto id = douyinapi::getID(url);
        EXPECT_EQ(id.type, douyinapi::IDType::AwemeId);
        EXPECT_FALSE(id.id.empty());
    }
}

TEST(DouYinUrlUnitTest, RejectsUnsupportedUrls)
{
    EXPECT_FALSE(douyinapi::isValidUrl("https://www.douyin.com/video/123"));
    EXPECT_FALSE(douyinapi::isValidUrl("https://live.douyin.com/123456"));
    EXPECT_EQ(douyinapi::getID("https://example.com/video/7000000000000000001").type, douyinapi::IDType::Unkown);
}

TEST(DouYinABogusUnitTest, SignsExactQueryAndBody)
{
    constexpr char userAgent[] = "Mozilla/5.0 test";
    constexpr uint64_t startTime = 1700000000000ULL;
    constexpr uint64_t endTime = startTime + 5;

    constexpr std::array<double, 3> randomValues = {17, 29, 41};
    const auto baseline = douyinapi::ABogus(userAgent, "").getValue("aid=6383&count=10", "cursor=0", startTime, endTime, randomValues);

    const auto sameInput = douyinapi::ABogus(userAgent, "").getValue("aid=6383&count=10", "cursor=0", startTime, endTime, randomValues);
    EXPECT_EQ(baseline, sameInput);
    EXPECT_FALSE(baseline.empty());

    const auto changedQuery = douyinapi::ABogus(userAgent, "").getValue("aid=6383&count=20", "cursor=0", startTime, endTime, randomValues);
    EXPECT_NE(baseline, changedQuery);

    const auto changedBody = douyinapi::ABogus(userAgent, "").getValue("aid=6383&count=10", "cursor=10", startTime, endTime, randomValues);
    EXPECT_NE(baseline, changedBody);
}

TEST(DouYinCryptoUnitTest, MatchesRc4AndSm3StandardVectors)
{
    const auto rc4 = crypto::rc4("Plaintext", "Key");
    EXPECT_EQ(encoding::hexEncode(rc4), "BBF316E8D940AF0AD3");

    EXPECT_EQ(encoding::hexEncode(crypto::sm3Raw("abc")), "66C7F0F462EEEDD9D1F2D46BDC10E4E24167C4875CF2F7A2297DA02B8F4BA8E0");
}
