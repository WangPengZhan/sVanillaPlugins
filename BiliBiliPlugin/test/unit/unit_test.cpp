#include <algorithm>
#include <array>
#include <ctime>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <string>

#include <curl/curl.h>
#include <gtest/gtest.h>
#include <openssl/evp.h>

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
constexpr char kRsaPublicKey[] = "-----BEGIN PUBLIC KEY-----\n"
                                 "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDgtQn2JZ34ZC28NWYpAUd98iZ3\n"
                                 "7BUrX/aKzmFbt7clFSs6sXqHauqKWqdtLkF2KexO40H1YTX8z2lSgBBOAxLsvakl\n"
                                 "V8k4cBFK9snQXE9/DDaFt6Rr7iVZMldczhC0JNgTz+SHXT6CBHuX3e9SdB1Ua44on\n"
                                 "caTWz7OBGLbCiK45wIDAQAB\n"
                                 "-----END PUBLIC KEY-----\n";

std::string legacyUrlEncode(const std::string& input)
{
    char* encoded = curl_easy_escape(nullptr, input.data(), static_cast<int>(input.size()));
    if (!encoded)
    {
        return {};
    }
    const std::string result(encoded);
    curl_free(encoded);
    return result;
}

std::string legacyMd5Hex(const std::string& input)
{
    unsigned char digest[EVP_MAX_MD_SIZE]{};
    unsigned int digestLength = 0;
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (!context)
    {
        return {};
    }

    const bool success = EVP_DigestInit_ex(context, EVP_md5(), nullptr) == 1 &&
                         EVP_DigestUpdate(context, input.data(), input.size()) == 1 &&
                         EVP_DigestFinal_ex(context, digest, &digestLength) == 1;
    EVP_MD_CTX_free(context);
    if (!success)
    {
        return {};
    }

    std::ostringstream output;
    for (unsigned int i = 0; i < digestLength; ++i)
    {
        output << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return output.str();
}
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
    EXPECT_TRUE(biliapi::typeToString(biliapi::IDType::Cid).empty());
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
    EXPECT_EQ(biliapi::GetMixinKey(kMixinInput), "UVsc1ixGpYkF6dTJBRfXHjQtDCoNmMPn");
    EXPECT_EQ(crypto::md5Hex("abc"), "900150983cd24fb0d6963f7d28e17f72");
    EXPECT_EQ(biliapi::hmac_sha256("key", "The quick brown fox jumps over the lazy dog"), "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");
}

TEST(BiliBiliUtilsUnitTest, SharedSigningHelpersMatchLegacyImplementations)
{
    const std::array inputs = {
        std::string(),
        std::string("bvid=BV19cLCzvErr&cid=123456&fnval=16&wts=1752910000"),
        std::string("a b+c?&value=!')(*"),
        std::string("Bilibili-\xE4\xB8\xAD\xE6\x96\x87", 16),
        std::string("\x00\x7f\x80\xff", 4),
    };

    for (const auto& input : inputs)
    {
        SCOPED_TRACE(::testing::PrintToString(input));
        EXPECT_EQ(encoding::urlEncode(input), legacyUrlEncode(input));
        EXPECT_EQ(crypto::md5Hex(input), legacyMd5Hex(input));
    }
}

TEST(BiliBiliUtilsUnitTest, CoversAllSharedEncodingHelpersWithFixedVectors)
{
    const std::string binary("hello\0", 6);
    EXPECT_EQ(encoding::base64Encode(binary), "aGVsbG8A");
    EXPECT_EQ(encoding::base64Decode("aGVsbG8A"), binary);
    EXPECT_EQ(encoding::hexEncode(std::string("\x00\x7f\x80\xff", 4)), "007F80FF");
    EXPECT_EQ(encoding::hexDecode("007F80FF"), std::string("\x00\x7f\x80\xff", 4));
    EXPECT_EQ(encoding::urlEncode("a b+c?"), "a%20b%2Bc%3F");
    EXPECT_EQ(encoding::urlDecode("a%20b%2Bc%3F"), "a b+c?");

    const std::vector<uint32_t> codePoints = {0x41, 0x4E2D, 0x1F600};
    const std::string utf8 = "A\xE4\xB8\xAD\xF0\x9F\x98\x80";
    EXPECT_EQ(encoding::utf8ToCodePoints(utf8), codePoints);
    EXPECT_EQ(encoding::codePointsToUtf8(codePoints), utf8);
}

TEST(BiliBiliUtilsUnitTest, CoversAllSharedCryptoHelpersWithFixedVectors)
{
    const auto ecbKey = encoding::hexDecode("000102030405060708090A0B0C0D0E0F");
    const auto ecbPlaintext = encoding::hexDecode("00112233445566778899AABBCCDDEEFF");
    const auto ecbCiphertext = crypto::aes128Encrypt(ecbPlaintext, "ecb", ecbKey, "", "hex");
    ASSERT_EQ(ecbCiphertext.size(), 64U);
    EXPECT_EQ(ecbCiphertext.substr(0, 32), "69C4E0D86A7B0430D8CDB78070B4C55A");
    EXPECT_EQ(crypto::aes128EcbDecrypt(ecbCiphertext, ecbKey, "", "hex"), ecbPlaintext);

    const auto cbcKey = encoding::hexDecode("2B7E151628AED2A6ABF7158809CF4F3C");
    const auto cbcIv = encoding::hexDecode("000102030405060708090A0B0C0D0E0F");
    const auto cbcPlaintext = encoding::hexDecode("6BC1BEE22E409F96E93D7E117393172A");
    const auto cbcCiphertext = crypto::aes128Encrypt(cbcPlaintext, "cbc", cbcKey, cbcIv, "hex");
    ASSERT_EQ(cbcCiphertext.size(), 64U);
    EXPECT_EQ(cbcCiphertext.substr(0, 32), "7649ABAC8119B246CEE98E9B12E9197D");

    EXPECT_EQ(crypto::rsaNoPaddingPublicEncryptHexLower(std::string("\x01", 1), kRsaPublicKey), std::string(254, '0') + "01");
    EXPECT_EQ(encoding::hexEncode(crypto::md5Raw("abc")), "900150983CD24FB0D6963F7D28E17F72");
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
