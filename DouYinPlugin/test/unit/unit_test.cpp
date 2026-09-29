#include <array>

#include <gtest/gtest.h>

#include "DouYinApi/DouYinUrl.h"
#include "DouYinApi/DouYinUtils.h"
#include "Plugin/Convert.h"
#include "Plugin/DouYinDownloader.h"

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

TEST(DouYinSeriesDetailUnitTest, ParsesCursorFieldsWithoutTruncation)
{
    const auto json = nlohmann::json::parse(
        R"({"aweme_list":[],"has_more":true,"cursor":1700000000000,"max_cursor":1700000000123,"min_cursor":1699999999000,"status_code":0})");
    const douyinapi::SeriesDetail detail = json.get<douyinapi::SeriesDetail>();

    EXPECT_TRUE(detail.has_more);
    EXPECT_EQ(detail.status_code, 0);
    EXPECT_EQ(detail.cursor, 1700000000000LL);
    EXPECT_EQ(detail.max_cursor, 1700000000123LL);
    EXPECT_EQ(detail.min_cursor, 1699999999000LL);
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

TEST(DouYinConvertUnitTest, SelectsCoverFallbacksAndHighestBitratePlayback)
{
    douyinapi::Video video;
    video.origin_cover.url_list = {"origin-cover"};
    video.cover_original_scale.url_list = {"scaled-cover"};
    video.cover.url_list = {"cover"};
    video.bit_rate.resize(3);
    video.bit_rate[0].bit_rate = 1000;
    video.bit_rate[0].play_addr.url_list = {"low"};
    video.bit_rate[1].bit_rate = 3000;
    video.bit_rate[1].play_addr.url_list = {"high"};
    video.bit_rate[2].bit_rate = 5000;
    video.play_addr.url_list = {"fallback"};

    EXPECT_EQ(getCover(video), "origin-cover");
    EXPECT_EQ(getPlayUrl(video), "high");

    video.origin_cover.url_list.clear();
    EXPECT_EQ(getCover(video), "scaled-cover");
    video.cover_original_scale.url_list.clear();
    EXPECT_EQ(getCover(video), "cover");
    video.cover.url_list.clear();
    video.cover.uri = "cover-uri";
    EXPECT_EQ(getCover(video), "cover-uri");

    video.bit_rate.clear();
    EXPECT_EQ(getPlayUrl(video), "fallback");
}

TEST(DouYinConvertUnitTest, FindsOwnerAndConvertsSeriesInOrder)
{
    douyinapi::FollowingResponse following;
    following.owner_sec_uid = "owner";
    following.followings = {
        {.nickname = "other",      .sec_uid = "other"},
        {.nickname = "owner name", .sec_uid = "owner"}
    };
    EXPECT_EQ(getUserInfo(following).nickname, "owner name");

    douyinapi::AwemeDetail first;
    first.aweme_id = "aweme-1";
    first.item_title = "first";
    first.author.nickname = "publisher";
    first.duration = 65;
    first.video.cover.uri = "cover-1";
    douyinapi::AwemeDetail second;
    second.aweme_id = "aweme-2";
    second.item_title = "second";

    douyinapi::SeriesDetail series;
    series.aweme_list = {first, second};
    const auto views = convertSeriesDetail(series);

    ASSERT_EQ(views.size(), 2);
    EXPECT_EQ(views[0].Identifier, "aweme-1");
    EXPECT_EQ(views[0].Title, "first");
    EXPECT_EQ(views[0].Publisher, "publisher");
    EXPECT_EQ(views[0].Cover, "cover-1");
    EXPECT_EQ(views[0].Duration, "01:05");
    EXPECT_EQ(views[0].pluginId, 8);
    EXPECT_EQ(views[1].Identifier, "aweme-2");
}

TEST(DouYinDownloaderUnitTest, PreservesResourceOutputWithoutStartingAria)
{
    download::ResourceInfo info;
    info.option.dir = "download-dir";
    info.option.out = "video.mp4";
    download::DouYinDownloader downloader(info);
    EXPECT_EQ(downloader.path(), "download-dir");
    EXPECT_EQ(downloader.filename(), "video.mp4");
    EXPECT_FALSE(downloader.isFinished());
}
