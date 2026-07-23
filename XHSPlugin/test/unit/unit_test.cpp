#include <array>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "XHSApi/XHSUrl.h"
#include "XHSApi/XHSUtils.h"
#include "Plugin/Convert.h"
#include "Plugin/XHSDownloader.h"

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

TEST(XHSUrlUnitTest, DoesNotExposeXsecTokenInDiagnosticText)
{
    const auto id = xhsapi::getID("https://www.xiaohongshu.com/explore/69abf6df0000000028009901?xsec_token=secret-token");
    const auto diagnostic = id.to_string();

    EXPECT_EQ(diagnostic.find("secret-token"), std::string::npos);
    EXPECT_NE(diagnostic.find("hasXsecToken=true"), std::string::npos);
}

TEST(XHSConvertUnitTest, SelectsHighestResolutionStreamAndFallsBackToBackupUrl)
{
    std::vector<xhsapi::StreamItem> streams(3);
    streams[0].height = 720;
    streams[0].video_bitrate = 1000;
    streams[0].master_url = "https://example.invalid/720.mp4";
    streams[1].height = 1080;
    streams[1].video_bitrate = 2000;
    streams[1].backup_urls = {"https://example.invalid/1080-backup.mp4"};
    streams[2].height = 1080;
    streams[2].video_bitrate = 1500;
    streams[2].master_url = "https://example.invalid/1080-low.mp4";

    EXPECT_EQ(getVideoUrl(streams), "https://example.invalid/1080-backup.mp4");
}

TEST(XHSDownloaderUnitTest, EmptyResourceFailsWithoutStartingAria)
{
    download::ResourceInfo info;
    download::XHSDownloader downloader(info);

    EXPECT_EQ(downloader.status(), download::AbstractDownloader::Ready);
    downloader.start();
    EXPECT_EQ(downloader.status(), download::AbstractDownloader::Error);
}

TEST(XHSSignerUnitTest, ParsesCookieHeaderWithoutLeakingValues)
{
    const auto cookies = xhsapi::parseSignCookies("a1=abc; web_session=session-value; empty=");
    EXPECT_EQ(cookies.at("a1"), "abc");
    EXPECT_EQ(cookies.at("web_session"), "session-value");
    EXPECT_TRUE(cookies.at("empty").empty());
}

TEST(XHSSignerUnitTest, XywMatchesUpstreamReferenceVector)
{
    const xhsapi::SignCookies cookies = {
        {"a1", std::string(52, 'a')},
        {"web_session", "session-value"}
    };
    const auto headers = xhsapi::signRequest("GET", "/api/sns/web/v1/user_posted?cursor=&num=10", "", cookies, xhsapi::SignFormat::Xyw, 1700000000123LL);

    EXPECT_EQ(headers.xT, "1700000000123");
    EXPECT_EQ(headers.xS,
              "XYW_"
              "eyJzaWduU3ZuIjoiNTYiLCJzaWduVHlwZSI6IngyIiwiYXBwSWQiOiJ4aHMtcGMtd2ViIiwic2lnblZlcnNpb24iOiIxIiwicGF5bG9hZCI6IjdhMWIzZGU5NWY4NWJjNDUwYTBhOWI4ZTg4"
              "NGU0NTJiOThiZTJiMWJlY2RjYTc0MzRjZDY1NDA3MDc2N2U0NzZkZGY5YTBjNTFiZGNlNTU2NWRhZjk3ZWQ1ZThmYTA2ZjMwNzk1NDRkODE0M2U1ZmE4MDVjMmE4NWEwMzA4YmNmOWNhNjgw"
              "NGJmNTdmYzY3ZDI4MDM5ZTE1NmViNDkwNGMzNTk2YTc4NGUzMGE5OThlZWQ2ZjViMzYzZGEyYTA1NWFiZWM0YzFjYzU3N2RiMTdmNTE3NjljMGFjOGI3OGMxMWQyM2FiYzQ5YzkxMGE3MTY3"
              "ZjZkN2ZkYTQ4ZDE3MjQyNmQ1OWZmMzNjOTY0NmFkNTkxMzQyZDJmNWQ4NGNlMDI1OGUxMGJjYTE2Y2E3OTYyOTliMDhjZWRmYTJkZTY0YmFkMmE3MTdmZjEzZjI3OGY4ZTFmYTgwNDdkYTlj"
              "OTYxZTNjN2VkY2Y4YjBmODgzM2YwZGY0NzEyZGE1NWY0ZjJhNWNiOGU5NmY3MWM3OTI5YzY0ZTNjM2Q1OWU4NDAyIn0=");
    EXPECT_FALSE(headers.xSCommon.empty());
}

TEST(XHSSignerUnitTest, XysProducesExpectedHeaderFamilies)
{
    const xhsapi::SignCookies cookies = {
        {"a1", std::string(52, 'a')}
    };
    const auto headers = xhsapi::signRequest("POST", "/api/sns/web/v1/feed", "{}", cookies, xhsapi::SignFormat::Xys, 1700000000123LL);

    EXPECT_EQ(headers.xS.rfind("XYS_", 0), 0U);
    EXPECT_FALSE(headers.xSCommon.empty());
    EXPECT_EQ(headers.xT, "1700000000123");
}
