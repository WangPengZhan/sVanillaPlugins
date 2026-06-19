#include <array>

#include <gtest/gtest.h>

#include "WeiboApi/WeiboUrl.h"

namespace
{
struct UrlCase
{
    const char* url;
    const char* id;
};

constexpr std::array<UrlCase, 10> kUrlCases = {
    {
     {"https://m.weibo.cn/status/N1a2b3c4D", "wid:N1a2b3c4D"},
     {"https://m.weibo.cn/detail/N2a2b3c4D", "wid:N2a2b3c4D"},
     {"https://weibo.com/1234567890/N3a2b3c4D", "wid:N3a2b3c4D"},
     {"https://www.weibo.com/1234567890/N4a2b3c4D", "wid:N4a2b3c4D"},
     {"https://weibo.com/0987654321/N5a2b3c4D", "wid:N5a2b3c4D"},
     {"https://video.weibo.com/show?fid=1034:abcdef1234567890", "mid:abcdef1234567890"},
     {"https://weibo.com/tv/show/1034:abcdef1234567891", "mid:abcdef1234567891"},
     {"https://h5.video.weibo.com/show/1034:abcdef1234567892", "mid:abcdef1234567892"},
     {"https://video.weibo.com/show?fid=1034:ABCDEF1234567893", "mid:ABCDEF1234567893"},
     {"https://weibo.com/tv/show/1034:ABCDEF1234567894", "mid:ABCDEF1234567894"},
     }
};
}  // namespace

TEST(WeiboUrlUnitTest, ExtractsSupportedUrlIds)
{
    for (const auto& testCase : kUrlCases)
    {
        SCOPED_TRACE(testCase.url);
        EXPECT_TRUE(isValidUrl(testCase.url));
        EXPECT_EQ(getID(testCase.url), testCase.id);
    }
}

TEST(WeiboUrlUnitTest, RejectsUnsupportedUrls)
{
    EXPECT_FALSE(isValidUrl("https://weibo.com/tv/show/1034:nothex"));
    EXPECT_TRUE(getID("https://example.com/1234567890/N1a2b3c4D").empty());
}
