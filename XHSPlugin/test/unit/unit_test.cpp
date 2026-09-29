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

TEST(XHSConvertUnitTest, PrefersMasterStreamThenFallsBackToBackupUrl)
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

    EXPECT_EQ(getVideoUrl(streams), "https://example.invalid/1080-low.mp4");

    streams[0].master_url.clear();
    streams[2].master_url.clear();
    EXPECT_EQ(getVideoUrl(streams), "https://example.invalid/1080-backup.mp4");
}

TEST(XHSConvertUnitTest, FallsBackAcrossCodecFamilies)
{
    xhsapi::Media media;
    media.stream.h264.emplace_back();
    media.stream.h265.resize(2);
    media.stream.h265[0].height = 720;
    media.stream.h265[0].master_url = "h265-720";
    media.stream.h265[1].height = 1080;
    media.stream.h265[1].master_url = "h265-1080";
    media.stream.av1.resize(1);
    media.stream.av1[0].height = 2160;
    media.stream.av1[0].master_url = "av1-2160";

    EXPECT_EQ(getVideoUrl(media), "h265-1080");
    media.stream.h265.clear();
    EXPECT_EQ(getVideoUrl(media), "av1-2160");
    media.stream.av1.clear();
    EXPECT_TRUE(getVideoUrl(media).empty());
}

TEST(XHSConvertUnitTest, ConvertsNoteCardsAndUserLists)
{
    xhsapi::NoteCard card;
    card.note_id = "note-card";
    card.title = "card title";
    card.user.nickname = "publisher";
    card.image_list = {{.url_default = "cover"}};
    card.video.capa.duration = 65;
    card.desc = "description";
    card.time = 1700000000000;

    const auto cardView = convertNoteDetail(card);
    EXPECT_EQ(cardView.Identifier, "note-card");
    EXPECT_EQ(cardView.Title, "card title");
    EXPECT_EQ(cardView.Publisher, "publisher");
    EXPECT_EQ(cardView.Cover, "cover");
    EXPECT_EQ(cardView.Duration, "01:05");
    EXPECT_EQ(cardView.Description, "description");
    EXPECT_FALSE(cardView.PublishDate.empty());
    EXPECT_EQ(cardView.pluginId, 7);

    xhsapi::NoteItemInfo first;
    first.note_id = "note-1";
    first.display_title = "first";
    first.xsec_token = "token-1";
    first.user.nickname = "user";
    first.cover.url_default = "first-cover";
    xhsapi::NoteItemInfo second;
    second.note_id = "note-2";
    xhsapi::NoteItemList list;
    list.notes = {first, second};

    const auto views = convertNoteDetail(list);
    ASSERT_EQ(views.size(), 2);
    EXPECT_EQ(views[0].Identifier, "note-1");
    EXPECT_EQ(views[0].Option1, "token-1");
    EXPECT_EQ(views[0].Title, "first");
    EXPECT_EQ(views[0].Publisher, "user");
    EXPECT_EQ(views[0].Cover, "first-cover");
    EXPECT_EQ(views[1].Identifier, "note-2");
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

TEST(XHSSignerUnitTest, XywIsDeterministicAndBindsRequestUri)
{
    const xhsapi::SignCookies cookies = {
        {"a1", std::string(52, 'a')},
        {"web_session", "session-value"}
    };
    const auto headers = xhsapi::signRequest("GET", "/api/sns/web/v1/user_posted?cursor=&num=10", "", cookies, xhsapi::SignFormat::Xyw, 1700000000123LL);
    const auto repeated = xhsapi::signRequest("GET", "/api/sns/web/v1/user_posted?cursor=&num=10", "", cookies, xhsapi::SignFormat::Xyw, 1700000000123LL);
    const auto changed = xhsapi::signRequest("GET", "/api/sns/web/v1/user_posted?cursor=next&num=10", "", cookies, xhsapi::SignFormat::Xyw, 1700000000123LL);

    EXPECT_EQ(headers.xT, "1700000000123");
    EXPECT_EQ(headers.xS.rfind("XYW_", 0), 0U);
    EXPECT_EQ(headers.xS, repeated.xS);
    EXPECT_NE(headers.xS, changed.xS);
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
