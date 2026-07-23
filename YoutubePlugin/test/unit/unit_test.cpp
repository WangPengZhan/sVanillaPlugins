#include <array>

#include <gtest/gtest.h>

#include "YoutubeApi/YoutubeConstants.h"
#include "YoutubeApi/YoutubeUrl.h"
#include "Plugin/Convert.h"

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

constexpr std::array<UrlCase, 10> kUrlCases = {
    {
     {"https://www.youtube.com/watch?v=abcdefghijk", "abcdefghijk", youtubeapi::IDType::VideoId},
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
