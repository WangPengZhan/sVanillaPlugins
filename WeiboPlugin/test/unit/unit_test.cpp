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

    weiboapi::WeiboAjaxData data{};
    data.mix_media_info.items = {image, first, second};
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
