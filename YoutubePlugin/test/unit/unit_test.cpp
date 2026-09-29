#include <array>

#include <gtest/gtest.h>

#include "YoutubeApi/YoutubeConstants.h"
#include "YoutubeApi/Cipher/CipherList.h"
#include "YoutubeApi/YoutubeUrl.h"
#include "Plugin/Convert.h"
#include "Plugin/YoutubeDownloader.h"

namespace
{
struct UrlCase
{
    const char* url;
    const char* id;
    youtubeapi::IDType type;
    const char* parentId = "";
    youtubeapi::IDType parentType = youtubeapi::IDType::Unkown;
};

constexpr std::array<UrlCase, 11> kUrlCases = {
    {
     {"https://www.youtube.com/watch?v=abcdefghijk", "abcdefghijk", youtubeapi::IDType::VideoId},
     {"https://youtube.com/watch?v=hijklmnopqr", "hijklmnopqr", youtubeapi::IDType::VideoId},
     {"https://music.youtube.com/watch?v=bcdefghijkl", "bcdefghijkl", youtubeapi::IDType::VideoId},
     {"https://www.youtubekids.com/watch?v=cdefghijklm", "cdefghijklm", youtubeapi::IDType::VideoId},
     {"https://youtu.be/defghijklmn", "defghijklmn", youtubeapi::IDType::VideoId},
     {"https://www.youtube.com/embed/efghijklmno", "efghijklmno", youtubeapi::IDType::VideoId},
     {"https://www.youtube.com/shorts/fghijklmnop", "fghijklmnop", youtubeapi::IDType::VideoId},
     {"https://www.youtube.com/playlist?list=PL0123456789ABCDEF", "PL0123456789ABCDEF", youtubeapi::IDType::PlaylistId},
     {"https://www.youtube.com/channel/UC0123456789ABCDE", "UC0123456789ABCDE", youtubeapi::IDType::ChannelId},
     {"https://www.youtube.com/@Sample_Channel", "Sample_Channel", youtubeapi::IDType::AtChannelId},
     {"https://www.youtube.com/watch?v=ghijklmnopq&list=PL0123456789ABCDEF", "ghijklmnopq", youtubeapi::IDType::VideoId, "PL0123456789ABCDEF",
         youtubeapi::IDType::PlaylistId},
     }
};
}  // namespace

TEST(YoutubeUrlUnitTest, ExtractsSupportedUrlIds)
{
    for (const auto& testCase : kUrlCases)
    {
        SCOPED_TRACE(testCase.url);
        EXPECT_TRUE(youtubeapi::isValidUrl(testCase.url));

        const auto id = youtubeapi::getID(testCase.url);
        EXPECT_EQ(id.id, testCase.id);
        EXPECT_EQ(id.type, testCase.type);
        EXPECT_EQ(id.parentId, testCase.parentId);
        EXPECT_EQ(id.parentType, testCase.parentType);
    }
}

TEST(YoutubeUrlUnitTest, ExtractsCustomChannelName)
{
    const auto id = youtubeapi::getID("https://www.youtube.com/c/SampleCustom");

    EXPECT_EQ(id.id, "SampleCustom");
    EXPECT_EQ(id.type, youtubeapi::IDType::CustomNameId);
}

TEST(YoutubeUrlUnitTest, ExtractsPartiallyShortenedVideoId)
{
    const auto id = youtubeapi::getID("https://youtu.be/watch?v=Fcds0_MrgNU");

    EXPECT_EQ(id.id, "Fcds0_MrgNU");
    EXPECT_EQ(id.type, youtubeapi::IDType::VideoId);
}

TEST(YoutubeUrlUnitTest, RejectsUnsupportedUrls)
{
    EXPECT_FALSE(youtubeapi::isValidUrl("https://www.youtube.com/watch"));
    EXPECT_EQ(youtubeapi::getID("https://example.com/watch?v=abcdefghijk").type, youtubeapi::IDType::Unkown);
    EXPECT_EQ(youtubeapi::getID("https://youtu.be/too-short").type, youtubeapi::IDType::Unkown);
}

TEST(YoutubeClientUnitTest, UsesAndroidVrPlayerIdentity)
{
    const auto request = nlohmann::json::parse(youtubeapi::youtubePostContent);
    const auto& client = request["context"]["client"];

    EXPECT_EQ(client["clientName"], "ANDROID_VR");
    EXPECT_EQ(client["clientVersion"], "1.60.19");
    EXPECT_EQ(client["deviceMake"], "Oculus");
    EXPECT_EQ(client["deviceModel"], "Quest 3");
    EXPECT_EQ(client["osName"], "Android");
    EXPECT_EQ(client["osVersion"], "12L");
    EXPECT_EQ(client["platform"], "MOBILE");
    EXPECT_EQ(youtubeapi::youtube_visitor_user_agent, "User-Agent: com.google.android.youtube/20.10.38 (Linux; U; ANDROID 11) gzip");
    EXPECT_EQ(youtubeapi::youtube_player_user_agent,
              "User-Agent: com.google.android.apps.youtube.vr.oculus/1.60.19 (Linux; U; Android 12L; Quest 3 Build/SQ3A.220605.009.A1) gzip");
    EXPECT_EQ(youtubeapi::youtube_default_cookies, "SOCS=CAISEwgDEgk4MTM4MzYzNTIaAmVuIAEaBgiApPzGBg; domain=.youtube.com");
    EXPECT_EQ(youtubeapi::swJsDataUrl, "https://www.youtube.com/sw.js_data?hl=en");
    EXPECT_EQ(youtubeapi::youtubePlayerApiKey, "AIzaSyA8eiZmM1FaDVjRy-df2KTyQ_vz_yYM39w");
}

TEST(YoutubeConvertUnitTest, PrioritizesSourceVideoWithoutReorderingOtherItems)
{
    adapter::VideoView views(3);
    views[0].Identifier = "first";
    views[1].Identifier = "selected";
    views[2].Identifier = "last";

    prioritizeVideoView(views, "selected");

    EXPECT_EQ(views[0].Identifier, "selected");
    EXPECT_EQ(views[1].Identifier, "first");
    EXPECT_EQ(views[2].Identifier, "last");

    prioritizeVideoView(views, "missing");
    EXPECT_EQ(views[0].Identifier, "selected");
    EXPECT_EQ(views[1].Identifier, "first");
    EXPECT_EQ(views[2].Identifier, "last");
}

TEST(YoutubeCipherUnitTest, AppliesReverseSpliceAndSwapOperations)
{
    youtubeapi::ReverseCipher reverse;
    EXPECT_EQ(reverse.decipher("abcdef"), "fedcba");
    EXPECT_EQ(reverse.to_string(), "Reverse");

    youtubeapi::SpliceCipher splice(2);
    EXPECT_EQ(splice.decipher("abcdef"), "cdef");
    EXPECT_EQ(splice.to_string(), "Splice 2");
    splice.setIndex(4);
    EXPECT_EQ(splice.index(), 4);
    EXPECT_EQ(splice.decipher("abcdef"), "ef");

    youtubeapi::SwapCipher swap(3);
    EXPECT_EQ(swap.decipher("abcdef"), "dbcaef");
    EXPECT_EQ(swap.to_string(), "Swap 3");
    swap.setIndex(99);
    EXPECT_EQ(swap.decipher("abcdef"), "abcdef");
    swap.setIndex(-1);
    EXPECT_EQ(swap.decipher("abcdef"), "abcdef");
}

TEST(YoutubeConvertUnitTest, ConvertsPlayerResponseAndHandlesInvalidDuration)
{
    youtubeapi::MainResponse response;
    response.videoDetails.videoId = "abcdefghijk";
    response.videoDetails.title = "video title";
    response.videoDetails.author = "publisher";
    response.videoDetails.lengthSeconds = "125";
    response.videoDetails.shortDescription = "description";
    response.videoDetails.thumbnail.thumbnails = {
        {.url = "small", .width = 120,  .height = 90 },
        {.url = "large", .width = 1280, .height = 720},
    };
    response.microformat.playerMicroformatRenderer.publishDate = "2026-07-25T12:34:56Z";

    auto views = convertVideoView(response);
    ASSERT_EQ(views.size(), 1);
    EXPECT_EQ(views[0].Identifier, "abcdefghijk");
    EXPECT_EQ(views[0].Title, "video title");
    EXPECT_EQ(views[0].Publisher, "publisher");
    EXPECT_EQ(views[0].Cover, "large");
    EXPECT_EQ(views[0].Duration, "02:05");
    EXPECT_EQ(views[0].Description, "description");
    EXPECT_EQ(views[0].PublishDate, "2026-07-25 12:34:56");
    EXPECT_EQ(views[0].pluginId, 2);

    response.videoDetails.lengthSeconds = "not-a-number";
    response.videoDetails.thumbnail.thumbnails.clear();
    response.microformat.playerMicroformatRenderer.publishDate = "invalid";
    views = convertVideoView(response);
    ASSERT_EQ(views.size(), 1);
    EXPECT_EQ(views[0].Duration, "0:00");
    EXPECT_TRUE(views[0].Cover.empty());
    EXPECT_TRUE(views[0].PublishDate.empty());
}

TEST(YoutubeConvertUnitTest, SkipsUnplayablePlayerResponse)
{
    youtubeapi::MainResponse response;
    response.videoDetails.videoId = "abcdefghijk";
    response.playabilityStatus.status = "ERROR";
    response.playabilityStatus.reason = "This video is not available";

    EXPECT_FALSE(response.isPlayable());
    EXPECT_TRUE(convertVideoView(response).empty());
}

TEST(YoutubeConvertUnitTest, ConvertsPlaylistItemMetadata)
{
    youtubeapi::PlaylistVideoRenderer renderer;
    renderer.videoId = "bcdefghijkl";
    renderer.title.runs = {{.text = "playlist title"}};
    renderer.shortBylineText.runs = {{.text = "author one"}, {.text = "author two"}};
    renderer.thumbnail.thumbnails = {
        {.url = "cover", .width = 320, .height = 180}
    };
    renderer.lengthText.simpleText = "3:21";

    const auto view = convertVideoInfo(renderer);
    EXPECT_EQ(view.Identifier, "bcdefghijkl");
    EXPECT_EQ(view.Title, "playlist title");
    EXPECT_EQ(view.Publisher, "author one;author two");
    EXPECT_EQ(view.Cover, "cover");
    EXPECT_EQ(view.Duration, "3:21");
    EXPECT_EQ(view.pluginId, 2);
}

TEST(YoutubeDownloaderUnitTest, PreservesResourceOutputWithoutStartingAria)
{
    download::ResourceInfo info;
    info.option.dir = "download-dir";
    info.option.out = "video.mp4";
    download::YoutubeDownloader downloader(info);
    EXPECT_EQ(downloader.path(), "download-dir");
    EXPECT_EQ(downloader.filename(), "video.mp4");
    EXPECT_FALSE(downloader.isFinished());
}
