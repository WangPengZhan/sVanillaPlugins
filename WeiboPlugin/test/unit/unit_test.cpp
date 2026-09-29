#include <array>

#include <gtest/gtest.h>

#include "WeiboApi/WeiboUrl.h"
#include "Plugin/Convert.h"
#include "Plugin/WeiboDownloader.h"

namespace
{
struct UrlCase
{
    const char* url;
    const char* id;
    weiboapi::IDType type;
};

constexpr std::array<UrlCase, 10> kUrlCases = {
    {
     {"https://m.weibo.cn/status/N1a2b3c4D", "N1a2b3c4D", weiboapi::IDType::Status},
     {"https://m.weibo.cn/detail/N2a2b3c4D", "N2a2b3c4D", weiboapi::IDType::Status},
     {"https://weibo.com/1234567890/N3a2b3c4D", "N3a2b3c4D", weiboapi::IDType::Status},
     {"https://www.weibo.com/1234567890/N4a2b3c4D", "N4a2b3c4D", weiboapi::IDType::Status},
     {"https://weibo.com/0987654321/N5a2b3c4D", "N5a2b3c4D", weiboapi::IDType::Status},
     {"https://video.weibo.com/show?fid=1034:abcdef1234567890", "abcdef1234567890", weiboapi::IDType::TV},
     {"https://weibo.com/tv/show/1034:abcdef1234567891", "abcdef1234567891", weiboapi::IDType::TV},
     {"https://h5.video.weibo.com/show/1034:abcdef1234567892", "abcdef1234567892", weiboapi::IDType::TV},
     {"https://video.weibo.com/show?fid=1034:ABCDEF1234567893", "ABCDEF1234567893", weiboapi::IDType::TV},
     {"https://weibo.com/tv/show/1034:ABCDEF1234567894", "ABCDEF1234567894", weiboapi::IDType::TV},
     }
};
}  // namespace

TEST(WeiboUrlUnitTest, ExtractsSupportedUrlIds)
{
    for (const auto& testCase : kUrlCases)
    {
        SCOPED_TRACE(testCase.url);
        EXPECT_TRUE(weiboapi::isValidUrl(testCase.url));

        const auto id = weiboapi::getID(testCase.url);
        EXPECT_EQ(id.id, testCase.id);
        EXPECT_EQ(id.type, testCase.type);
    }
}

TEST(WeiboUrlUnitTest, RejectsUnsupportedUrls)
{
    EXPECT_FALSE(weiboapi::isValidUrl("https://weibo.com/tv/show/1034:nothex"));

    const auto id = weiboapi::getID("https://example.com/1234567890/N1a2b3c4D");
    EXPECT_TRUE(id.id.empty());
    EXPECT_EQ(id.type, weiboapi::IDType::Unkown);
}

TEST(WeiboUrlUnitTest, ClassifiesIdsWithoutStringPrefixes)
{
    const auto status = weiboapi::getID("https://weibo.com/1234567890/N1a2b3c4D");
    const auto tv = weiboapi::getID("https://weibo.com/tv/show/1034:abcdef1234567891");

    // The id is returned with the "1034:" fid prefix already stripped for TV.
    EXPECT_EQ(status.id, "N1a2b3c4D");
    EXPECT_EQ(tv.id, "abcdef1234567891");
    EXPECT_NE(status.type, tv.type);
    EXPECT_NE(weiboapi::typeToString(status.type), weiboapi::typeToString(tv.type));
}

TEST(WeiboUrlUnitTest, MapsTypesToAndFromTheirNames)
{
    EXPECT_EQ(weiboapi::typeToString(weiboapi::IDType::Status), "Status");
    EXPECT_EQ(weiboapi::typeToString(weiboapi::IDType::TV), "TV");
    EXPECT_EQ(weiboapi::typeToString(weiboapi::IDType::Unkown), "");

    EXPECT_EQ(weiboapi::stringToType("Status"), weiboapi::IDType::Status);
    EXPECT_EQ(weiboapi::stringToType("TV"), weiboapi::IDType::TV);
    EXPECT_EQ(weiboapi::stringToType("unknown-type"), weiboapi::IDType::Unkown);

    EXPECT_EQ(weiboapi::stringToType(weiboapi::typeToString(weiboapi::IDType::Status)), weiboapi::IDType::Status);
    EXPECT_EQ(weiboapi::stringToType(weiboapi::typeToString(weiboapi::IDType::TV)), weiboapi::IDType::TV);
}

TEST(WeiboUrlUnitTest, AcceptsIdsWithTrailingQueryOrFragment)
{
    EXPECT_TRUE(weiboapi::isValidUrl("https://m.weibo.cn/status/N1a2b3c4D?from=feed&loc=avatar"));
    EXPECT_EQ(weiboapi::getID("https://m.weibo.cn/status/N1a2b3c4D?from=feed").id, "N1a2b3c4D");

    EXPECT_TRUE(weiboapi::isValidUrl("https://video.weibo.com/show?fid=1034:abcdef1234567890&from=app#/"));
    EXPECT_EQ(weiboapi::getID("https://video.weibo.com/show?fid=1034:abcdef1234567890&from=app").id, "abcdef1234567890");

    EXPECT_FALSE(weiboapi::isValidUrl("https://evil.com/redirect?u=https://m.weibo.cn/status/N1a2b3c4D"));
}

TEST(WeiboConvertUnitTest, ConvertsComponentPlaybackMetadata)
{
    weiboapi::ComponentPlayPlayinfoResponse response{};
    auto& playInfo = response.data.Component_Play_Playinfo;
    playInfo.id = "video-id";
    playInfo.media_id = 987654321;
    playInfo.title = "video title";
    playInfo.author = "publisher";
    playInfo.cover_image = "//example.invalid/cover.jpg";
    playInfo.duration_time = 125;
    playInfo.text = "description";
    playInfo.real_date = 1700000000;

    const auto views = convertVideoView(response);

    ASSERT_EQ(views.size(), 1);
    EXPECT_EQ(views[0].Identifier, "video-id");
    EXPECT_EQ(views[0].Option2, "987654321");
    EXPECT_EQ(views[0].Title, "video title");
    EXPECT_EQ(views[0].Publisher, "publisher");
    EXPECT_EQ(views[0].Cover, "https://example.invalid/cover.jpg");
    EXPECT_EQ(views[0].Duration, "02:05");
    EXPECT_EQ(views[0].Description, "description");
    EXPECT_FALSE(views[0].PublishDate.empty());
    EXPECT_EQ(views[0].pluginId, 4);
}

TEST(WeiboConvertUnitTest, DropsPlaybackResponseWithoutId)
{
    weiboapi::ComponentPlayPlayinfoResponse response{};
    response.data.Component_Play_Playinfo.title = "ghost title";

    EXPECT_TRUE(convertVideoView(response).empty());
}

TEST(WeiboConvertUnitTest, FiltersNonVideoMixedMediaAndPreservesVideoOrder)
{
    weiboapi::MediaInfoItem image{};
    image.type = "pic";
    image.id = "image-id";

    weiboapi::MediaInfoItem first{};
    first.type = "video";
    first.id = "video-1";
    first.data.page_id = "page-1";
    first.data.content2 = "first description";
    first.data.media_info.media_id = "media-1";
    first.data.media_info.name = "first ";
    first.data.media_info.author_name = "author";
    first.data.media_info.duration = 65;
    first.data.media_info.big_pic_info.pic_big.url = "first-cover";

    weiboapi::MediaInfoItem second{};
    second.type = "video_collection";
    second.id = "video-2";
    second.data.media_info.media_id = "media-2";
    second.data.media_info.big_pic_info.pic_big.url = "//example.invalid/cover-2.jpg";

    weiboapi::MediaInfoItem withoutId{};
    withoutId.type = "video";
    withoutId.data.media_info.media_id = "media-3";

    weiboapi::WeiboAjaxData data{};
    data.mix_media_info.items = {image, first, second, withoutId};
    const auto views = convertVideoView(data);

    ASSERT_EQ(views.size(), 2);
    EXPECT_EQ(views[0].Identifier, "video-1");
    EXPECT_EQ(views[0].Option1, "page-1");
    EXPECT_EQ(views[0].Option2, "media-1");
    EXPECT_EQ(views[0].Title, "first media-1");
    EXPECT_EQ(views[0].Publisher, "author");
    EXPECT_EQ(views[0].Cover, "first-cover");
    EXPECT_EQ(views[0].Duration, "01:05");
    EXPECT_EQ(views[0].Description, "first description");
    EXPECT_EQ(views[0].pluginId, 4);
    EXPECT_EQ(views[1].Identifier, "video-2");
    EXPECT_EQ(views[1].Cover, "https://example.invalid/cover-2.jpg");
}

TEST(WeiboDownloaderUnitTest, SetVideoUrisReplacesReportedUris)
{
    download::WeiboDownloader downloader;
    downloader.setVideoUris({"https://example.invalid/video.mp4"});

    ASSERT_EQ(downloader.uris().size(), 1U);
    EXPECT_EQ(downloader.uris().front(), "https://example.invalid/video.mp4");
}

TEST(WeiboDownloaderUnitTest, PreservesResourceOutputWithoutStartingAria)
{
    download::ResourceInfo info;
    info.option.dir = "download-dir";
    info.option.out = "video.mp4";
    download::WeiboDownloader downloader(info);
    EXPECT_EQ(downloader.path(), "download-dir");
    EXPECT_EQ(downloader.filename(), "video.mp4");
    EXPECT_FALSE(downloader.isFinished());
}
