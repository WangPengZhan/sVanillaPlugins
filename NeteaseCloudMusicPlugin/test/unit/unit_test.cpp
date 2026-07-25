#include <array>

#include <gtest/gtest.h>

#include "NeteaseCloudMusicApi/NeteaseCloudMusicUrl.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicUtils.h"
#include "Plugin/Convert.h"
#include "Plugin/NeteaseCloudMusicDownloader.h"

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
}

TEST(NeteaseCloudMusicUtilsUnitTest, ParsesAndSerializesCookieValues)
{
    const auto cookies = netease::cookieToJson(" MUSIC_U=token value ; os=pc; empty=; invalid ");

    ASSERT_EQ(cookies.size(), 3);
    EXPECT_EQ(cookies.at("MUSIC_U"), "token value");
    EXPECT_EQ(cookies.at("os"), "pc");
    EXPECT_TRUE(cookies.at("empty").empty());

    const auto serialized = netease::cookieObjToString(cookies);
    EXPECT_NE(serialized.find("MUSIC_U=token%20value"), std::string::npos);
    EXPECT_NE(serialized.find("os=pc"), std::string::npos);
    EXPECT_NE(serialized.find("empty="), std::string::npos);
    EXPECT_EQ(netease::concatenateCurlHeader("X-Real-IP", "116.25.1.2"), "X-Real-IP: 116.25.1.2\r\n");
}

TEST(NeteaseCloudMusicUtilsUnitTest, ShapesDeterministicEapiRequest)
{
    const nlohmann::ordered_json body = {
        {"ids",   nlohmann::ordered_json::array({347230})},
        {"level", "standard"                             }
    };

    const auto first = netease::eapi("/api/song/enhance/player/url/v1", body);
    const auto second = netease::eapi("/api/song/enhance/player/url/v1", body);
    const auto changed = netease::eapi("/api/song/enhance/player/url/v1", {
                                                                              {"ids", nlohmann::ordered_json::array({347231})}
    });

    ASSERT_TRUE(first.contains("params"));
    EXPECT_EQ(first.at("params"), second.at("params"));
    EXPECT_NE(first.at("params"), changed.at("params"));
    EXPECT_FALSE(first.at("params").empty());
    EXPECT_EQ(first.at("params").size() % 32, 0);
}

TEST(NeteaseCloudMusicConvertUnitTest, ConvertsSongsAndPreservesOrder)
{
    netease::Song first{};
    first.id = 347230;
    first.name = "first song";
    first.ar = {
        {.id = 1, .name = "artist one"},
        {.id = 2, .name = "artist two"}
    };
    first.al.picUrl = "https://example.invalid/cover.jpg";
    first.dt = 125000;
    first.rt = "description";
    first.publishTime = 1700000000000;
    netease::Song second{};
    second.id = 347231;
    second.name = "second song";

    netease::SongDetails details{};
    details.songs = {first, second};
    const auto views = convertVideoView(details);

    ASSERT_EQ(views.size(), 2);
    EXPECT_EQ(views[0].Identifier, "347230");
    EXPECT_EQ(views[0].Title, "first song");
    EXPECT_EQ(views[0].Publisher, "artist one;artist two");
    EXPECT_EQ(views[0].Cover, first.al.picUrl);
    EXPECT_EQ(views[0].Duration, "02:05");
    EXPECT_EQ(views[0].Description, first.rt);
    EXPECT_EQ(views[0].fileExtension, ".mp3");
    EXPECT_EQ(views[0].pluginId, 5);
    EXPECT_EQ(views[1].Identifier, "347231");
}

TEST(NeteaseCloudMusicDownloaderUnitTest, PreservesResourceOutputWithoutStartingAria)
{
    download::ResourceInfo info;
    info.option.dir = "download-dir";
    info.option.out = "audio.mp3";
    download::NeteaseCloudMusicDownloader downloader(info);
    EXPECT_EQ(downloader.path(), "download-dir");
    EXPECT_EQ(downloader.filename(), "audio.mp3");
    EXPECT_FALSE(downloader.isFinished());
}
