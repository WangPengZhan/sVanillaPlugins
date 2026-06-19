#include <array>

#include <gtest/gtest.h>

#include "YoutubeApi/YoutubeUrl.h"

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

TEST(YoutubeUrlUnitTest, RejectsUnsupportedUrls)
{
    EXPECT_FALSE(youtubeapi::isValidUrl("https://www.youtube.com/watch"));
    EXPECT_EQ(youtubeapi::getID("https://example.com/watch?v=abcdefghijk").type, youtubeapi::IDType::Unkown);
}
