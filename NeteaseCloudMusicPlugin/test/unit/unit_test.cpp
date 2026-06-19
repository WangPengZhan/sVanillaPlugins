#include <array>

#include <gtest/gtest.h>

#include "NeteaseCloudMusicApi/NeteaseCloudMusicUrl.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicUtils.h"

namespace
{
struct UrlCase
{
    const char* url;
    const char* id;
};

constexpr std::array<UrlCase, 10> kUrlCases = {
    {
     {"https://music.163.com/song?id=347230", "song:347230"},
     {"https://music.163.com/#/song?id=347231", "song:347231"},
     {"https://music.163.com/m/song?id=347232", "song:347232"},
     {"https://y.music.163.com/song?id=347233", "song:347233"},
     {"https://music.163.com/album?id=28521", "album:28521"},
     {"https://music.163.com/#/album?id=28522", "album:28522"},
     {"https://music.163.com/artist?id=6452", "artist:6452"},
     {"https://music.163.com/#/artist?id=6453", "artist:6453"},
     {"https://music.163.com/playlist?id=3778678", "playlist:3778678"},
     {"https://music.163.com/mv?id=5436712", "mv:5436712"},
     }
};
}  // namespace

TEST(NeteaseCloudMusicUrlUnitTest, ExtractsSupportedUrlIds)
{
    for (const auto& testCase : kUrlCases)
    {
        SCOPED_TRACE(testCase.url);
        EXPECT_TRUE(isValidUrl(testCase.url));
        EXPECT_EQ(getID(testCase.url), testCase.id);
    }
}

TEST(NeteaseCloudMusicUrlUnitTest, ExtractsToplistAsPlaylist)
{
    constexpr char url[] = "https://music.163.com/discover/toplist?id=19723756";

    EXPECT_TRUE(isValidUrl(url));
    EXPECT_EQ(getID(url), "playlist:19723756");
}

TEST(NeteaseCloudMusicUrlUnitTest, RejectsUnsupportedUrls)
{
    EXPECT_FALSE(isValidUrl("https://music.163.com/song"));
    EXPECT_TRUE(getID("https://example.com/song?id=347230").empty());
}

TEST(NeteaseCloudMusicUnitTest, KeepsLocalCryptoHelpersDeterministic)
{
    constexpr char id[] = "5EB26EA49AE37590BF1618A40D227A1E286F98F4AF0F89AE4565";

    EXPECT_EQ(netease::cloudMusicDllEncodeID(id), "n2ajvcolCXIBkynlYKPukA==");
    EXPECT_EQ(netease::base64Encode(std::string(id) + " n2ajvcolCXIBkynlYKPukA=="),
              "NUVCMjZFQTQ5QUUzNzU5MEJGMTYxOEE0MEQyMjdBMUUyODZGOThGNEFGMEY4OUFFNDU2NSBuMmFqdmNvbENYSUJreW5sWUtQdWtBPT0=");
}
