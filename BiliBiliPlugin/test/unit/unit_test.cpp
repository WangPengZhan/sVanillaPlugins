#include <algorithm>
#include <array>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <curl/curl.h>
#include <gtest/gtest.h>
#include <openssl/evp.h>

#include "PluginCrypto/Crypto.h"
#include "PluginCrypto/Crc32.h"
#include "PluginCrypto/Encoding.h"
#include "Aria2Net/AriaClient/AriaClient.h"
#include "BiliApi/BilibiliClient.h"
#include "BiliApi/BilibiliUrl.h"
#include "BiliApi/BilibiliUtils.h"
#include "Plugin/Convert.h"
#include "LoggerRegisterHelpper.h"
#include "LoginProxy.h"
#include "FFmpeg/FFmpegHelper.h"
#include "Util/TimerUtil.h"
#include "Util/process.hpp"

namespace
{
const std::filesystem::path kRuntimeDir = "bili-unit-test/runtime";

std::filesystem::path ariaExecutableName()
{
#ifdef _WIN32
    return "aria2c.exe";
#else
    return "aria2c";
#endif
}

TinyProcessLib::Process::string_type toProcessString(const std::filesystem::path& value)
{
#if defined(_WIN32) && defined(UNICODE)
    return value.wstring();
#else
    return value.string();
#endif
}

bool ariaIsRunning()
{
    return !aria2net::AriaClient::globalClient().GetAriaVersionAsync().result.version.empty();
}

class AriaTestEnvironment final : public testing::Environment
{
public:
    void SetUp() override
    {
        LoggerRegisterHelpper::registerLogger("Aria2Net", kRuntimeDir.string() + "/log/Aria2Net.log");
        LoggerRegisterHelpper::registerLogger("FFmpeg", kRuntimeDir.string() + "/log/FFmpeg.log");
        LoggerRegisterHelpper::registerLogger("Network", kRuntimeDir.string() + "/log/Network.log");
        LoggerRegisterHelpper::registerLogger("Download", kRuntimeDir.string() + "/log/Download.log");
        LoggerRegisterHelpper::registerLogger("BiliBili", kRuntimeDir.string() + "/log/BiliBili.log");

        if (ariaIsRunning())
        {
            return;
        }

        const auto exeDir = std::filesystem::path(getModulePath());
        const auto ariaExecutable = exeDir / "aria" / ariaExecutableName();
        ASSERT_TRUE(std::filesystem::exists(ariaExecutable)) << "Cannot find aria2c in test output aria directory";

        const auto ariaDir = exeDir / kRuntimeDir / "aria";
        std::filesystem::create_directories(ariaDir);
        const auto sessionFile = ariaDir / "aria.session";
        std::ofstream(sessionFile, std::ios::app).close();
        std::vector<TinyProcessLib::Process::string_type> args = {
            toProcessString(ariaExecutable),
            toProcessString("--enable-rpc"),
            toProcessString("--rpc-listen-all=false"),
            toProcessString("--rpc-listen-port=6800"),
            toProcessString("--rpc-secret=sVanilla"),
            toProcessString("--input-file=" + sessionFile.string()),
            toProcessString("--save-session=" + sessionFile.string()),
            toProcessString("--log=" + (ariaDir / "aria.log").string()),
            toProcessString("--max-concurrent-downloads=6"),
            toProcessString("--max-connection-per-server=16"),
            toProcessString("--split=5"),
            toProcessString("--continue=true"),
            toProcessString("--allow-overwrite=true"),
            toProcessString("--auto-file-renaming=false"),
            toProcessString("--file-allocation=none"),
        };
        TinyProcessLib::Config config;
        config.show_window = TinyProcessLib::Config::ShowWindow::hide;
        aria2Process_ = std::make_unique<TinyProcessLib::Process>(args, toProcessString(exeDir), nullptr, nullptr, false, config);

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (std::chrono::steady_clock::now() < deadline && !ariaIsRunning())
        {
            int exitStatus = 0;
            ASSERT_FALSE(aria2Process_->try_get_exit_status(exitStatus)) << "aria2c exited during startup";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        ASSERT_TRUE(ariaIsRunning()) << "aria2c RPC did not become ready within five seconds";
    }

    void TearDown() override
    {
        if (aria2Process_)
        {
            aria2Process_->kill(true);
        }
    }

private:
    std::unique_ptr<TinyProcessLib::Process> aria2Process_;
};

testing::Environment* const kAriaTestEnvironment = testing::AddGlobalTestEnvironment(new AriaTestEnvironment);

class FakeLoginApi final : public AbstractLoginApi
{
public:
    bool supportsLogin() const override
    {
        return true;
    }
    bool isLoggedIn() const override
    {
        return loggedIn;
    }
    std::string cookies() const override
    {
        return cookieValue;
    }
    void setCookies(std::string cookies) override
    {
        cookieValue = std::move(cookies);
    }
    bool refreshCookies(std::string cookies) override
    {
        cookieValue = std::move(cookies);
        return true;
    }
    bool logout() override
    {
        loggedIn = false;
        return true;
    }
    std::string domain() const override
    {
        return "https://www.bilibili.com";
    }
    UserInfo getUserInfo(std::string dir) override
    {
        lastDirectory = std::move(dir);
        return user;
    }
    std::vector<adapter::BaseVideoView> history() override
    {
        return historyViews;
    }
    int pluginId() const override
    {
        return 1;
    }
    LoginStatus getLoginStatus() override
    {
        return LoginStatus::Success;
    }
    bool getScanContext(std::string& content) override
    {
        content = "scan";
        return true;
    }
    void loginSuccess() override
    {
        loggedIn = true;
    }
    const LoginResource& allResources() const override
    {
        return resources;
    }
    const std::vector<uint8_t>& resource(ResourceIndex index) const override
    {
        return resources.at(index);
    }

    bool loggedIn = true;
    std::string cookieValue = "initial";
    std::string lastDirectory;
    UserInfo user{.uname = "tester", .id = "42"};
    std::vector<adapter::BaseVideoView> historyViews = {adapter::BaseVideoView{.Identifier = "BV-history"}};
    LoginResource resources{};
};

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

    const bool success = EVP_DigestInit_ex(context, EVP_md5(), nullptr) == 1 && EVP_DigestUpdate(context, input.data(), input.size()) == 1 &&
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
    EXPECT_EQ(encoding::hexEncode(crypto::sm3Raw("abc")), "66C7F0F462EEEDD9D1F2D46BDC10E4E24167C4875CF2F7A2297DA02B8F4BA8E0");
    EXPECT_EQ(encoding::hexEncode(crypto::rc4("Plaintext", "Key")), "BBF316E8D940AF0AD3");
    EXPECT_EQ(crypto::rc4(crypto::rc4("Plaintext", "Key"), "Key"), "Plaintext");
    EXPECT_EQ(crypto::md5Hex(""), "d41d8cd98f00b204e9800998ecf8427e");
    EXPECT_EQ(biliapi::hmac_sha256("key", "The quick brown fox jumps over the lazy dog"), "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");
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

TEST(BiliBiliConvertUnitTest, PrioritizesSourceVideoWithoutReorderingOtherItems)
{
    adapter::VideoView views(3);
    views[0].Identifier = "first";
    views[1].Identifier = "selected";
    views[2].Identifier = "last";

    biliapi::IDInfo source;
    source.id = "selected";
    source.type = biliapi::IDType::Bid;
    prioritizeVideoView(views, source);

    EXPECT_EQ(views[0].Identifier, "selected");
    EXPECT_EQ(views[1].Identifier, "first");
    EXPECT_EQ(views[2].Identifier, "last");
}

TEST(BiliBiliConvertUnitTest, PrioritizesAidAndLeavesMissingSourceUnchanged)
{
    adapter::VideoView views(2);
    views[0].Identifier = "BV-first";
    views[0].Option1 = "100";
    views[1].Identifier = "BV-selected";
    views[1].Option1 = "200";

    biliapi::IDInfo source;
    source.id = "200";
    source.type = biliapi::IDType::Aid;
    prioritizeVideoView(views, source);
    EXPECT_EQ(views[0].Identifier, "BV-selected");

    source.id = "missing";
    prioritizeVideoView(views, source);
    EXPECT_EQ(views[0].Identifier, "BV-selected");
    EXPECT_EQ(views[1].Identifier, "BV-first");
}

TEST(PluginCommonUnitTest, SerializesConfigurationViewsAndPluginMetadata)
{
    const auto config = nlohmann::json::parse(R"({"downloadDir":"output","videoQuality":"R1080P","nameRule":"$title$-$id$"})").get<DownloadConfig>();
    EXPECT_EQ(config.downloadDir, "output");
    EXPECT_EQ(config.videoQuality, VideQuality::R1080P);
    EXPECT_EQ(config.nameRule, "$title$-$id$");

    adapter::BaseVideoView view;
    view.Identifier = "BV-test";
    view.Title = "title";
    view.fileType = adapter::FileType::Audio;
    view.pluginId = 1;
    const auto restoredView = nlohmann::json(view).get<adapter::BaseVideoView>();
    EXPECT_EQ(restoredView.Identifier, view.Identifier);
    EXPECT_EQ(restoredView.Title, view.Title);
    EXPECT_EQ(restoredView.fileType, view.fileType);
    EXPECT_EQ(restoredView.pluginId, view.pluginId);

    const PluginMessage message{1, "BiliBili", "1.0", "description", "https://www.bilibili.com"};
    EXPECT_EQ(nlohmann::json(message).get<PluginMessage>().domain, message.domain);
}

TEST(PluginCommonUnitTest, ResolvesVideoNamesGuidAndNullInputs)
{
    VideoInfoFull empty;
    EXPECT_TRUE(empty.getGuid().empty());
    EXPECT_EQ(empty.parseNameRules("literal"), "literal");

    VideoInfoFull info;
    info.downloadConfig = std::make_shared<DownloadConfig>();
    info.downloadConfig->downloadDir = "downloads";
    info.downloadConfig->videoQuality = VideQuality::R720P;
    info.downloadConfig->nameRule = "$publisher$-$title$-$id$-$publishdate$";
    info.videoView = std::make_shared<adapter::BaseVideoView>();
    info.videoView->Identifier = "BV123";
    info.videoView->Title = "title:with?invalid*characters";
    info.videoView->Publisher = "publisher";
    info.videoView->PublishDate = "2026-07-23";
    info.videoView->IdType = "Bid";
    info.videoView->fileExtension = ".mp4";

    EXPECT_EQ(info.fileName(), "publisher-titlewithinvalidcharacters-BV123-2026-07-23.mp4");
    EXPECT_FALSE(info.getGuid().empty());
    EXPECT_EQ(info.coverPath(), info.getGuid());
    EXPECT_EQ(VideoInfoFull::ruleList.size(), VideoInfoFull::showRuleList.size());
    EXPECT_EQ(VideoInfoFull::ruleMap.at(adapter::title), "$title$");
}

TEST(BiliBiliConvertUnitTest, DetectsAndConvertsSingleAndMultipartVideos)
{
    biliapi::VideoView source;
    source.aid = 42;
    source.bvid = "BV-single";
    source.cid = 84;
    source.title = "single";
    source.owner.name = "owner";
    source.duration = 65;
    source.desc = "description";
    EXPECT_FALSE(checkSeason(source));
    EXPECT_FALSE(checkPages(source));

    const auto single = convertSingleVideo(source);
    EXPECT_EQ(single.Identifier, "BV-single");
    EXPECT_EQ(single.IdType, "Bid");
    EXPECT_EQ(single.Option1, "42");
    EXPECT_EQ(single.Option2, "84");
    EXPECT_EQ(single.Duration, "01:05");
    EXPECT_EQ(single.Publisher, "owner");
    EXPECT_EQ(single.pluginId, 1);

    biliapi::VideoPage first;
    first.cid = 101;
    first.part = "part one";
    first.duration = 30;
    biliapi::VideoPage second = first;
    second.cid = 102;
    second.part = "part two";
    source.pages = {first, second};
    EXPECT_TRUE(checkPages(source));
    const auto pages = convertVideoView(source);
    ASSERT_EQ(pages.size(), 2U);
    EXPECT_EQ(pages[0].Identifier, "BV-single");
    EXPECT_EQ(pages[1].Option2, "102");
}

TEST(BiliBiliConvertUnitTest, ConvertsBangumiCheeseFavoriteAndUserItems)
{
    biliapi::Episode bangumi;
    bangumi.ep_id = 11;
    bangumi.aid = 12;
    bangumi.cid = 13;
    bangumi.bvid = "BV-bangumi";
    bangumi.title = "episode";
    bangumi.duration = 120000;
    const auto bangumiView = convertEpisodes(bangumi);
    EXPECT_EQ(bangumiView.Identifier, "11");
    EXPECT_EQ(bangumiView.IdType, "BangumiEP");
    EXPECT_EQ(bangumiView.Option3, "BV-bangumi");

    biliapi::CheeseEpisode cheese;
    cheese.id = 21;
    cheese.aid = 22;
    cheese.cid = 23;
    cheese.title = "lesson";
    cheese.duration = 90;
    const auto cheeseView = convertEpisodes(cheese);
    EXPECT_EQ(cheeseView.Identifier, "21");
    EXPECT_EQ(cheeseView.IdType, "CheeseEP");
    EXPECT_EQ(cheeseView.Option2, "23");

    biliapi::FavVideoInfo favorite;
    favorite.bvid = "BV-favorite";
    favorite.ugc.first_cid = 31;
    favorite.title = "favorite";
    favorite.upper.name = "upper";
    const auto favoriteView = convertVideoInfo(favorite);
    EXPECT_EQ(favoriteView.Option2, "31");
    EXPECT_EQ(favoriteView.Publisher, "upper");

    biliapi::VlistItem userVideo;
    userVideo.bvid = "BV-user";
    userVideo.title = "user video";
    userVideo.length = "01:23";
    userVideo.author = "author";
    const auto userView = convertVideoInfo(userVideo);
    EXPECT_EQ(userView.Duration, "01:23");
    EXPECT_EQ(userView.Publisher, "author");
}

TEST(BiliBiliUrlUnitTest, AcceptsEverySupportedHostAndPathVariant)
{
    const std::array<std::pair<std::string, biliapi::IDType>, 12> cases = {
        {
         {"http://bilibili.com/av170001", biliapi::IDType::Aid},
         {"https://b23.tv/av170001", biliapi::IDType::Aid},
         {"https://bilibili.com/BV1xx411c7mD", biliapi::IDType::Bid},
         {"http://b23.tv/video/BV1xx411c7mD", biliapi::IDType::Bid},
         {"http://bilibili.com/bangumi/play/ss1", biliapi::IDType::BangumiSS},
         {"https://bilibili.com/bangumi/play/ep2", biliapi::IDType::BangumiEP},
         {"https://bilibili.com/bangumi/media/md3", biliapi::IDType::BangumiMD},
         {"https://bilibili.com/cheese/play/ss4", biliapi::IDType::CheeseSS},
         {"https://bilibili.com/cheese/play/ep5", biliapi::IDType::CheeseEP},
         {"https://bilibili.com/medialist/detail/ml6", biliapi::IDType::FavoritesId},
         {"https://bilibili.com/medialist/play/ml7", biliapi::IDType::FavoritesId},
         {"http://space.bilibili.com/8", biliapi::IDType::UserId},
         }
    };
    for (const auto& [url, type] : cases)
    {
        SCOPED_TRACE(url);
        EXPECT_TRUE(biliapi::isValidUrl(url));
        EXPECT_EQ(biliapi::getID(url).type, type);
    }
}

TEST(BiliBiliUrlUnitTest, RejectsMalformedIdsHostsAndPaths)
{
    const std::array invalidUrls = {
        "",
        "not-a-url",
        "ftp://www.bilibili.com/video/av1",
        "https://example.com/video/BV1xx411c7mD",
        "https://www.bilibili.com/video/av",
        "https://www.bilibili.com/video/avabc",
        "https://www.bilibili.com/video/BVshort",
        "https://www.bilibili.com/video/bv1xx411c7mD",
        "https://www.bilibili.com/bangumi/play/ssabc",
        "https://www.bilibili.com/cheese/play/ep",
        "https://space.bilibili.com/user",
        "https://www.bilibili.com/read/cv1",
    };
    for (const auto* url : invalidUrls)
    {
        SCOPED_TRACE(url);
        EXPECT_FALSE(biliapi::isValidUrl(url));
        const auto id = biliapi::getID(url);
        EXPECT_TRUE(id.id.empty());
        EXPECT_EQ(id.type, biliapi::IDType::Unkown);
    }
}

TEST(BiliBiliUtilsUnitTest, HandlesReplacementFilteringAndHashEdgeCases)
{
    std::string repeated = "aaaa";
    biliapi::replaceCharacter(repeated, "aa", "b");
    EXPECT_EQ(repeated, "bb");
    biliapi::replaceCharacter(repeated, "missing", "x");
    EXPECT_EQ(repeated, "bb");

    std::string removed = "a--b--";
    biliapi::replaceCharacter(removed, "--", "");
    EXPECT_EQ(removed, "ab");
    EXPECT_EQ(biliapi::filterCharacters("!a'b(c)d*"), "abcd");
    EXPECT_TRUE(biliapi::filterCharacters("!'()*").empty());
    EXPECT_EQ(biliapi::hmac_sha256("", ""), "b613679a0814d9ec772f95d778c35fc5ff1697c493715653c6c712144292c5ad");
}

TEST(BiliBiliUtilsUnitTest, HandlesCookieDirectorySeparators)
{
    biliapi::setCookieDataDir("D:/data/");
    EXPECT_EQ(biliapi::cookieDataFilePath(), "D:/data/sVanilla.data");
    biliapi::setCookieDataDir("D:\\data\\");
    EXPECT_EQ(biliapi::cookieDataFilePath(), "D:\\data\\sVanilla.data");
    biliapi::setCookieDataDir("");
}

TEST(PluginCommonUnitTest, GeneratesAndUsesEveryNameRule)
{
    VideoInfoFull info;
    info.downloadConfig = std::make_shared<DownloadConfig>();
    info.videoView = std::make_shared<adapter::BaseVideoView>();
    info.videoView->Identifier = "BV-id";
    info.videoView->Title = "title:with?invalid*characters";
    info.videoView->Publisher = "publisher/name";
    info.videoView->PublishDate = "2026-07-23";
    info.videoView->fileExtension = ".mp4";
    info.dateTimeResolver.date = "2026-07-23";
    info.dateTimeResolver.time = "12-34-56";
    info.dateTimeResolver.dateTime = "2026-07-23_12-34-56";

    const std::array<std::pair<std::string, std::string>, 10> cases = {
        {
         {"$id$", "BV-id.mp4"},
         {"$title$", "titlewithinvalidcharacters.mp4"},
         {"$publisher$", "publishername.mp4"},
         {"$publishdate$", "2026-07-23.mp4"},
         {"$date$", "2026-07-23.mp4"},
         {"$time$", "12-34-56.mp4"},
         {"$datetime$", "2026-07-23_12-34-56.mp4"},
         {"$id$$title$", "BV-idtitlewithinvalidcharacters.mp4"},
         {"$id$-$id$-literal", "BV-id-BV-id-literal.mp4"},
         {"prefix-$publisher$-$title$-suffix", "prefix-publishername-titlewithinvalidcharacters-suffix.mp4"},
         }
    };
    for (const auto& [rule, expected] : cases)
    {
        SCOPED_TRACE(rule);
        EXPECT_EQ(info.parseNameRules(rule), expected);
    }

    EXPECT_EQ(info.parseNameRules("$unknown$"), "$unknown$.mp4");
    EXPECT_EQ(info.parseNameRules(""), ".mp4");

    info.videoView->fileExtension = ".m4a";
    EXPECT_EQ(info.parseNameRules("$id$"), "BV-id.m4a");
    info.videoView->fileExtension.clear();
    EXPECT_EQ(info.parseNameRules("$id$"), "BV-id");

    DateTimeResolver resolver;
    resolver.generator();
    EXPECT_EQ(resolver.date.size(), 10U);
    EXPECT_EQ(resolver.time.size(), 8U);
    EXPECT_EQ(resolver.dateTime, resolver.date + "_" + resolver.time);
}

TEST(PluginCommonUnitTest, GuidReflectsDownloadIdentityFields)
{
    VideoInfoFull first;
    first.downloadConfig = std::make_shared<DownloadConfig>();
    first.videoView = std::make_shared<adapter::BaseVideoView>();
    first.downloadConfig->downloadDir = "one";
    first.downloadConfig->nameRule = "$id$";
    first.videoView->Identifier = "BV1";
    first.videoView->IdType = "Bid";
    first.videoView->Option1 = "aid";
    first.videoView->Option2 = "cid";
    const auto original = first.getGuid();
    ASSERT_FALSE(original.empty());

    auto second = first;
    second.downloadConfig = std::make_shared<DownloadConfig>(*first.downloadConfig);
    second.videoView = std::make_shared<adapter::BaseVideoView>(*first.videoView);
    second.downloadConfig->videoQuality = VideQuality::R1080P;
    EXPECT_NE(second.getGuid(), original);
    second.downloadConfig->videoQuality = first.downloadConfig->videoQuality;
    second.videoView->Option3 = "changed";
    EXPECT_NE(second.getGuid(), original);
}

TEST(PluginCommonUnitTest, LoginProxyDelegatesCompleteApiContract)
{
    FakeLoginApi login;
    LoginProxy proxy(login);
    EXPECT_EQ(proxy.loginWay(), AbstractLogin::LoginWay::Api);
    EXPECT_EQ(&proxy.realLogin(), &login);
    EXPECT_TRUE(proxy.supportsLogin());
    EXPECT_TRUE(proxy.isLoggedIn());
    EXPECT_EQ(proxy.cookies(), "initial");
    proxy.setCookies("set");
    EXPECT_EQ(login.cookieValue, "set");
    EXPECT_TRUE(proxy.refreshCookies("refreshed"));
    EXPECT_EQ(proxy.cookies(), "refreshed");
    EXPECT_EQ(proxy.domain(), "https://www.bilibili.com");
    EXPECT_EQ(proxy.pluginId(), 1);
    EXPECT_EQ(proxy.getUserInfo("dir").uname, "tester");
    EXPECT_EQ(login.lastDirectory, "dir");
    ASSERT_EQ(proxy.history().size(), 1U);
    EXPECT_EQ(proxy.history().front().Identifier, "BV-history");
    EXPECT_TRUE(proxy.logout());
    EXPECT_FALSE(proxy.isLoggedIn());
}

TEST(BiliBiliConvertUnitTest, ConvertsUgcSeasonAndFallsBackToPageDescription)
{
    biliapi::UgcEpisode episode;
    episode.bvid = "BV-season";
    episode.aid = 10;
    episode.cid = 20;
    episode.title = "episode";
    episode.page.duration = 61;
    episode.page.part = "page description";
    biliapi::UgcSection section;
    section.episodes.push_back(episode);
    biliapi::VideoView source;
    source.owner.name = "owner";
    source.ugc_season.title = "playlist";
    source.ugc_season.sections.push_back(section);

    EXPECT_TRUE(checkSeason(source));
    const auto views = convertVideoView(source);
    ASSERT_EQ(views.size(), 1U);
    EXPECT_EQ(views.front().Description, "page description");
    EXPECT_EQ(views.front().Publisher, "owner");
    EXPECT_EQ(views.front().PlayListTitle, "playlist");
}

TEST(BiliBiliConvertUnitTest, ConvertsHistoryAndCoverFallbacks)
{
    biliapi::HistoryInfo first;
    first.history.bvid = "BV-history-1";
    first.history.cid = 101;
    first.title = "first";
    first.cover = "primary-cover";
    first.duration = 65;
    first.new_desc = "description";
    biliapi::HistoryInfo second = first;
    second.history.bvid = "BV-history-2";
    second.cover.clear();
    second.covers = {"fallback-cover"};
    biliapi::HistoryInfo third = second;
    third.history.bvid = "BV-history-3";
    third.covers.clear();

    EXPECT_EQ(convertHistory(first).Cover, "primary-cover");
    EXPECT_EQ(convertHistory(second).Cover, "fallback-cover");
    EXPECT_TRUE(convertHistory(third).Cover.empty());
    biliapi::History history;
    history.data.list = {first, second, third};
    EXPECT_EQ(convertVideoView(history).size(), 3U);
}

TEST(BiliBiliConvertUnitTest, ConvertsBangumiCheeseAndUserCollections)
{
    biliapi::Episode mainEpisode;
    mainEpisode.ep_id = 1;
    biliapi::Episode sectionEpisode;
    sectionEpisode.ep_id = 2;
    biliapi::Section section;
    section.episodes.push_back(sectionEpisode);
    biliapi::BangumiData bangumi;
    bangumi.episodes.push_back(mainEpisode);
    bangumi.section.push_back(section);
    bangumi.up_info.uname = "bangumi publisher";
    const auto bangumiViews = convertVideoView(bangumi);
    ASSERT_EQ(bangumiViews.size(), 2U);
    EXPECT_EQ(bangumiViews[0].Publisher, "bangumi publisher");
    EXPECT_EQ(bangumiViews[1].Identifier, "2");

    biliapi::CheeseEpisode lesson;
    lesson.id = 3;
    biliapi::CheeseInfo cheese;
    cheese.episodes.push_back(lesson);
    cheese.up_info.uname = "teacher";
    const auto cheeseViews = convertVideoView(cheese);
    ASSERT_EQ(cheeseViews.size(), 1U);
    EXPECT_EQ(cheeseViews.front().Publisher, "teacher");

    biliapi::VlistItem item;
    item.bvid = "BV-user-list";
    biliapi::VideoWorks works;
    works.list.vlist.push_back(item);
    ASSERT_EQ(convertVideoView(works).size(), 1U);
    EXPECT_EQ(convertVideoView(works).front().Identifier, "BV-user-list");
}

TEST(PluginCommonCryptoTest, CoversCrcAndBinaryEncodingFailureBoundaries)
{
    EXPECT_EQ(checksum::crc32("123456789"), 0xCBF43926U);
    EXPECT_EQ(checksum::crc32(""), 0U);
    // EXPECT_TRUE(encoding::hexDecode("not-hex").empty());
    EXPECT_TRUE(encoding::base64Decode("***").empty());
}

TEST(PluginCommonNetworkTest, PerformsARealJsonRequest)
{
    network::NetWork client;
    std::string response;
    ASSERT_TRUE(client.get("https://jsonplaceholder.typicode.com/todos/1", response));
    ASSERT_FALSE(response.empty());
    const auto json = nlohmann::json::parse(response);
    EXPECT_EQ(json.at("id").get<int>(), 1);
    EXPECT_EQ(json.at("userId").get<int>(), 1);
    EXPECT_TRUE(json.at("title").is_string());
    EXPECT_TRUE(json.at("completed").is_boolean());
}

TEST(PluginCommonFFmpegTest, InvokesBundledFfmpegAndReportsCallbacks)
{
    bool failed = false;
    bool finished = false;
    const std::vector<std::string> arguments = {"-version"};
    const auto result = ffmpeg::FFmpegHelper::globalInstance().startFFmpeg(
        arguments,
        [&failed] {
            failed = true;
        },
        [&finished] {
            finished = true;
        });
    EXPECT_TRUE(result);
    EXPECT_FALSE(failed);
    EXPECT_TRUE(finished);
}

TEST(BiliBiliClientRequestTest, RequestsVideoViewAndVideoPlayback)
{
    auto& client = biliapi::BilibiliClient::globalClient();
    const auto view = client.getVideoView("BV19cLCzvErr", biliapi::IDType::Bid);
    ASSERT_EQ(view.code, 0);
    ASSERT_GT(view.data.cid, 0);
    EXPECT_EQ(view.data.bvid, "BV19cLCzvErr");

    const auto playback = client.getPlayUrl(view.data.cid, 64, view.data.bvid, 16);
    EXPECT_EQ(playback.code, 0);
}

TEST(BiliBiliClientRequestTest, RequestsBangumiSeasonEpisodeMediaAndPlayback)
{
    auto& client = biliapi::BilibiliClient::globalClient();
    const auto season = client.getSeasonVideoView("38233", biliapi::IDType::BangumiSS);
    ASSERT_EQ(season.code, 0);
    ASSERT_FALSE(season.result.episodes.empty());
    const auto episodeId = season.result.episodes.front().ep_id;

    const auto episode = client.getSeasonVideoView(std::to_string(episodeId), biliapi::IDType::BangumiEP);
    EXPECT_EQ(episode.code, 0);
    const auto media = client.getMdVideoView("5843");
    EXPECT_EQ(media.code, 0);
    const auto playback = client.getPlayUrl(episodeId, biliapi::IDType::BangumiEP, 64, 16);
    EXPECT_EQ(playback.code, 0);
}

TEST(BiliBiliClientRequestTest, RequestsCheeseSeasonEpisodeAndPlayback)
{
    auto& client = biliapi::BilibiliClient::globalClient();
    const auto season = client.getCheeseVideoView("523935492", biliapi::IDType::CheeseSS);
    ASSERT_EQ(season.code, 0);
    ASSERT_FALSE(season.data.episodes.empty());
    const auto& first = season.data.episodes.front();

    const auto episode = client.getCheeseVideoView(std::to_string(first.id), biliapi::IDType::CheeseEP);
    EXPECT_EQ(episode.code, 0);
    const auto playback = client.getPlayUrl(first.aid, first.id, first.cid, 64, 16);
    EXPECT_EQ(playback.code, 0);
}

TEST(BiliBiliClientRequestTest, RequestsFavoriteInterfaces)
{
    auto& client = biliapi::BilibiliClient::globalClient();
    constexpr auto kMediaId = "1103407912";
    const auto info = client.getFavInfo(kMediaId);
    EXPECT_TRUE(info.code == 0 || info.code == -1);
    const auto detail = client.getFavDetail(kMediaId);
    EXPECT_TRUE(detail.code == 0 || detail.code == -1);
    const auto videos = client.getFavVideoInfo(detail.data, 84912, 1103407912);
    EXPECT_TRUE(videos.code == 0 || videos.code == -1);
    const auto created = client.getCreatedFavList(20, 1, 84912);
    EXPECT_GE(created.count, 0);
    const auto collected = client.getCollectFavList(20, 1, 84912);
    EXPECT_GE(collected.count, 0);
}

TEST(BiliBiliClientRequestTest, RequestsUserWorks)
{
    auto& client = biliapi::BilibiliClient::globalClient();
    const auto works = client.getUserVideoWroks("2");
    EXPECT_TRUE(works.code == 0 || works.code == -1);
    if (works.code == 0)
    {
        EXPECT_FALSE(works.data.is_risk);
    }
}

TEST(BiliBiliClientRequestTest, RequestsWebBootstrapAndLoginInterfaces)
{
    auto& client = biliapi::BilibiliClient::globalClient();
    EXPECT_EQ(client.getTicket().code, 0);
    EXPECT_EQ(client.getBuvidInfo().code, 0);
    const auto buvid34 = client.getBuvid34Info();
    EXPECT_TRUE(buvid34.code == 0 || buvid34.code == -1);

    const auto loginUrl = client.getLoginUrl();
    ASSERT_EQ(loginUrl.code, 0);
    ASSERT_FALSE(loginUrl.data.qrcode_key.empty());
    const auto loginStatus = client.getLoginStatus(loginUrl.data.qrcode_key);
    EXPECT_EQ(loginStatus.code, 0);
    const auto nav = client.getNavInfo();
    EXPECT_TRUE(nav.code == 0 || nav.code == -101);
}
