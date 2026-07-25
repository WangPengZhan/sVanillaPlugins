#include <array>

#include <gtest/gtest.h>

#include "DedaoApi/DedaoUrl.h"
#include "Plugin/Convert.h"
#include "Plugin/DedaoDownloader.h"

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

TEST(DedaoConvertUnitTest, ConvertsLiveMetadataAndTeachers)
{
    dedaoapi::LiveDetail detail;
    detail.title = "live title";
    detail.logo = "https://example.invalid/live.jpg";
    detail.playback_duration_text = "01:02:03";
    detail.summary = "live summary";
    detail.last_end_time = "2026-07-25 10:00:00";
    detail.live_lecture_list = {{.teacher_name = "teacher one"}, {.teacher_name = "teacher two"}};

    const auto views = convertVideoView(detail);

    ASSERT_EQ(views.size(), 1);
    EXPECT_EQ(views[0].Title, detail.title);
    EXPECT_EQ(views[0].Publisher, "teacher one;teacher two");
    EXPECT_EQ(views[0].Cover, detail.logo);
    EXPECT_EQ(views[0].Duration, detail.playback_duration_text);
    EXPECT_EQ(views[0].Description, detail.summary);
    EXPECT_EQ(views[0].PublishDate, detail.last_end_time);
    EXPECT_EQ(views[0].pluginId, 6);
}

TEST(DedaoConvertUnitTest, ConvertsArticleDownloadMetadata)
{
    dedaoapi::Article article{};
    article.enid = "article-id";
    article.class_enid = "course-id";
    article.audio.mp3_play_url = "https://example.invalid/article.mp3";
    article.audio.duration = 125;
    article.title = "article title";
    article.logo = "https://example.invalid/article.jpg";
    article.summary = "article summary";
    article.update_time = 1700000000;

    const auto view = convertVideoView(article);

    EXPECT_EQ(view.Identifier, article.enid);
    EXPECT_EQ(view.ParentId, article.class_enid);
    EXPECT_EQ(view.Option1, article.audio.mp3_play_url);
    EXPECT_EQ(view.Title, article.title);
    EXPECT_EQ(view.Cover, article.logo);
    EXPECT_EQ(view.Duration, "02:05");
    EXPECT_EQ(view.Description, article.summary);
    EXPECT_FALSE(view.PublishDate.empty());
    EXPECT_EQ(view.pluginId, 6);
}

TEST(DedaoDownloaderUnitTest, PreservesResourceOutputWithoutStartingAria)
{
    download::ResourceInfo info;
    info.option.dir = "download-dir";
    info.option.out = "video.mp4";
    download::DedaoDownloader downloader(info);
    EXPECT_EQ(downloader.path(), "download-dir");
    EXPECT_EQ(downloader.filename(), "video.mp4");
    EXPECT_FALSE(downloader.isFinished());
}
