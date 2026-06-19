#include <array>

#include <gtest/gtest.h>

#include "DedaoApi/DedaoUrl.h"

namespace
{
struct UrlCase
{
    const char* url;
    const char* id;
    IDType type;
};

constexpr std::array<UrlCase, 10> kUrlCases = {
    {
     {"https://www.dedao.cn/live/detail?id=abcdefghijklmnopqrst", "abcdefghijklmnopqrst", IDType::Live},
     {"https://dedao.cn/live/detail?id=ABCDEFGHIJKLMNOPQRST1234", "ABCDEFGHIJKLMNOPQRST1234", IDType::Live},
     {"https://www.dedao.cn/ebook/reader?id=ebook10001", "ebook10001", IDType::EBook},
     {"https://dedao.cn/ebook/reader?id=EBOOK20002", "EBOOK20002", IDType::EBook},
     {"https://www.dedao.cn/course/detail?id=course30003", "course30003", IDType::Course},
     {"https://dedao.cn/course/detail?id=COURSE40004", "COURSE40004", IDType::Course},
     {"https://www.dedao.cn/course/article?id=article50005", "article50005", IDType::Article},
     {"https://dedao.cn/course/article?id=ARTICLE60006", "ARTICLE60006", IDType::Article},
     {"https://www.dedao.cn/course/detail?id=abc123XYZ789", "abc123XYZ789", IDType::Course},
     {"https://www.dedao.cn/course/article?id=abc123XYZ789", "abc123XYZ789", IDType::Article},
     }
};
}  // namespace

TEST(DedaoUrlUnitTest, ExtractsSupportedUrlIds)
{
    for (const auto& testCase : kUrlCases)
    {
        SCOPED_TRACE(testCase.url);
        EXPECT_TRUE(isValidUrl(testCase.url));

        const auto id = getID(testCase.url);
        EXPECT_EQ(id.id, testCase.id);
        EXPECT_EQ(id.type, testCase.type);
    }
}

TEST(DedaoUrlUnitTest, RejectsUnsupportedUrls)
{
    EXPECT_FALSE(isValidUrl("https://www.dedao.cn/live/detail?id=short"));
    EXPECT_EQ(getID("https://example.com/course/detail?id=course30003").type, IDType::Unkown);
}
