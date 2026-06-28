#include <array>
#include <ctime>
#include <string>

#include <gtest/gtest.h>

#include "PluginCrypto/Crypto.h"
#include "PluginCrypto/Encoding.h"
#include "BiliApi/BilibiliClient.h"
#include "BiliApi/BilibiliUrl.h"
#include "BiliApi/BilibiliUtils.h"

namespace
{
template <typename Urls>
void expectCases(const Urls& urls, const char* expectedId, biliapi::IDType expectedType)
{
    for (const auto& url : urls)
    {
        SCOPED_TRACE(url);
        EXPECT_TRUE(biliapi::isValidUrl(url));

        const auto id = biliapi::getID(url);
        EXPECT_EQ(id.id, expectedId);
        EXPECT_EQ(id.type, expectedType);
    }
}

template <std::size_t N>
std::array<std::string, N> addQuery(const char* base)
{
    std::array<std::string, N> urls{};
    for (std::size_t i = 0; i < N; ++i)
    {
        urls[i] = std::string(base) + "?case=" + std::to_string(i);
    }
    return urls;
}

constexpr char kMixinInput[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-/";
}  // namespace

TEST(BiliBiliUrlUnitTest, ExtractsAidUrls)
{
    const auto urls = addQuery<10>("https://www.bilibili.com/video/av170001");
    expectCases(urls, "170001", biliapi::IDType::Aid);
}

TEST(BiliBiliUrlUnitTest, ExtractsBidUrls)
{
    const auto urls = addQuery<10>("https://www.bilibili.com/video/BV1xx411c7mD");
    expectCases(urls, "1xx411c7mD", biliapi::IDType::Bid);
}

TEST(BiliBiliUrlUnitTest, ExtractsBangumiSsUrls)
{
    const auto urls = addQuery<10>("https://www.bilibili.com/bangumi/play/ss90001");
    expectCases(urls, "90001", biliapi::IDType::BangumiSS);
}

TEST(BiliBiliUrlUnitTest, ExtractsBangumiEpUrls)
{
    const auto urls = addQuery<10>("https://www.bilibili.com/bangumi/play/ep90002");
    expectCases(urls, "90002", biliapi::IDType::BangumiEP);
}

TEST(BiliBiliUrlUnitTest, ExtractsBangumiMdUrls)
{
    const auto urls = addQuery<10>("https://www.bilibili.com/bangumi/media/md90003");
    expectCases(urls, "90003", biliapi::IDType::BangumiMD);
}

TEST(BiliBiliUrlUnitTest, ExtractsCheeseSsUrls)
{
    const auto urls = addQuery<10>("https://www.bilibili.com/cheese/play/ss80001");
    expectCases(urls, "80001", biliapi::IDType::CheeseSS);
}

TEST(BiliBiliUrlUnitTest, ExtractsCheeseEpUrls)
{
    const auto urls = addQuery<10>("https://www.bilibili.com/cheese/play/ep80002");
    expectCases(urls, "80002", biliapi::IDType::CheeseEP);
}

TEST(BiliBiliUrlUnitTest, ExtractsFavoritesUrls)
{
    const auto urls = addQuery<10>("https://www.bilibili.com/medialist/detail/ml70001");
    expectCases(urls, "70001", biliapi::IDType::FavoritesId);
}

TEST(BiliBiliUrlUnitTest, ExtractsUserUrls)
{
    const auto urls = addQuery<10>("https://space.bilibili.com/60001");
    expectCases(urls, "60001", biliapi::IDType::UserId);
}

TEST(BiliBiliUrlUnitTest, RejectsUnsupportedUrls)
{
    EXPECT_FALSE(biliapi::isValidUrl("https://www.bilibili.com/read/cv123"));
    EXPECT_EQ(biliapi::getID("https://example.com/video/av170001").type, biliapi::IDType::Unkown);
}

TEST(BiliBiliUrlUnitTest, ConvertsTypesAndFormatsIds)
{
    EXPECT_EQ(biliapi::typeToString(biliapi::IDType::Aid), "Aid");
    EXPECT_EQ(biliapi::typeToString(biliapi::IDType::Cid), "Cid");
    EXPECT_EQ(biliapi::typeToString(biliapi::IDType::FavoritesId), "FavoritesId");
    EXPECT_EQ(biliapi::stringToType("BangumiEP"), biliapi::IDType::BangumiEP);
    EXPECT_EQ(biliapi::stringToType("UserId"), biliapi::IDType::UserId);
    EXPECT_EQ(biliapi::stringToType("missing"), biliapi::IDType::Unkown);

    biliapi::IDInfo info;
    info.id = "170001";
    info.type = biliapi::IDType::Aid;
    info.parentId = "90001";
    info.parentType = biliapi::IDType::BangumiSS;

    const auto text = info.to_string();
    EXPECT_NE(text.find("170001"), std::string::npos);
    EXPECT_NE(text.find("Aid"), std::string::npos);
    EXPECT_NE(text.find("90001"), std::string::npos);
}

TEST(BiliBiliUtilsUnitTest, ReplaceCharacterAndFilterCharacters)
{
    std::string text = "a-b-c";
    biliapi::replaceCharacter(text, "-", "_");
    EXPECT_EQ(text, "a_b_c");

    EXPECT_EQ(biliapi::filterCharacters("a!b'c(d)*"), "abcd");
}

TEST(BiliBiliUtilsUnitTest, EncodesAndDecodesUrls)
{
    const auto encoded = encoding::urlEncode("a b+c?");
    EXPECT_EQ(encoded, "a%20b%2Bc%3F");
    EXPECT_EQ(encoding::urlDecode(encoded), "a b+c?");
}

TEST(BiliBiliUtilsUnitTest, BuildsMixinKeyAndHashes)
{
    EXPECT_EQ(biliapi::GetMixinKey(kMixinInput), "UVsc1ixGpYkF6dTJBRfXHjQtDCoNmMPnLWhqy3O9Arab8ZEewz2v47g-5+lKuIS0");
    EXPECT_EQ(crypto::md5Hex("abc"), "900150983cd24fb0d6963f7d28e17f72");
    EXPECT_EQ(biliapi::hmac_sha256("key", "The quick brown fox jumps over the lazy dog"),
              "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");
}

TEST(BiliBiliUtilsUnitTest, HandlesCookiePathsAndExpiry)
{
    biliapi::setCookieDataDir("");
    EXPECT_EQ(biliapi::cookieDataFilePath(), "sVanilla.data");

    biliapi::setCookieDataDir("D:/temp");
    EXPECT_EQ(biliapi::cookieDataFilePath(), "D:/temp/sVanilla.data");
    biliapi::setCookieDataDir("");

    const auto today = std::time(nullptr) / 86400;
    EXPECT_TRUE(biliapi::isExpired(today - 1));
    EXPECT_FALSE(biliapi::isExpired(today + 1));
}

TEST(BiliBiliUtilsUnitTest, TypeConversionRoundTrips)
{
    EXPECT_EQ(biliapi::typeToString(biliapi::IDType::Aid), "Aid");
    EXPECT_EQ(biliapi::typeToString(biliapi::IDType::BangumiSS), "BangumiSS");
    EXPECT_EQ(biliapi::stringToType("Bid"), biliapi::IDType::Bid);
    EXPECT_EQ(biliapi::stringToType("CheeseEP"), biliapi::IDType::CheeseEP);
    EXPECT_EQ(biliapi::stringToType("not-a-type"), biliapi::IDType::Unkown);
}

TEST(BiliBiliUtilsUnitTest, FormatsHistoryQueryParameters)
{
    EXPECT_EQ(biliapi::to_string(biliapi::BusinessType::Archive), "archive");
    EXPECT_EQ(biliapi::to_string(biliapi::BusinessType::Article_list), "article-list");
    EXPECT_EQ(biliapi::to_string(biliapi::HistoryType::Live), "live");
    EXPECT_EQ(biliapi::to_string(biliapi::HistoryType::All), "all");

    biliapi::HistoryQueryParam param;
    param.max = 20;
    param.business = biliapi::BusinessType::Live;
    param.view_at = 123456789;
    param.type = biliapi::HistoryType::Article;
    param.ps = 30;

    const auto text = param.toString();
    EXPECT_NE(text.find("max: 20"), std::string::npos);
    EXPECT_NE(text.find("business: live"), std::string::npos);
    EXPECT_NE(text.find("view_at: 123456789"), std::string::npos);
    EXPECT_NE(text.find("type: article"), std::string::npos);
    EXPECT_NE(text.find("ps: 30"), std::string::npos);
}
