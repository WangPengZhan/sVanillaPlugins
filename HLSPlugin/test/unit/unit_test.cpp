#include <array>

#include <gtest/gtest.h>

#include "HLSApi/HLSApi.h"
#include "HLSApi/HLSParser.h"

namespace
{
constexpr std::array<const char*, 10> kExpectedSegmentUris = {
    "https://stream.example.com/live/master/seg-001.ts",
    "https://stream.example.com/live/master/video/seg-002.ts",
    "https://stream.example.com/live/shared/seg-003.ts",
    "https://stream.example.com/absolute/seg-004.ts",
    "https://cdn.example.com/hls/seg-005.ts",
    "http://cdn.example.com/hls/seg-006.ts",
    "https://stream.example.com/live/master/720p/seg-007.ts",
    "https://stream.example.com/live/master/audio/seg-008.aac",
    "https://stream.example.com/live/master/subtitles/seg-009.vtt",
    "https://stream.example.com/live/master/seg-010.ts?token=abc",
};
}

TEST(HLSParserUnitTest, ParsesMediaPlaylistWithTenSegmentUris)
{
    constexpr char playlist[] = "#EXTM3U\n"
                                "#EXT-X-TARGETDURATION:8\n"
                                "#EXT-X-KEY:METHOD=AES-128,URI=\"keys/key.bin\",IV=0x00000000000000000000000000000001\n"
                                "#EXTINF:4.0,segment 1\n"
                                "seg-001.ts\n"
                                "#EXTINF:4.1,segment 2\n"
                                "video/seg-002.ts\n"
                                "#EXTINF:4.2,segment 3\n"
                                "../shared/seg-003.ts\n"
                                "#EXTINF:4.3,segment 4\n"
                                "/absolute/seg-004.ts\n"
                                "#EXTINF:4.4,segment 5\n"
                                "https://cdn.example.com/hls/seg-005.ts\n"
                                "#EXTINF:4.5,segment 6\n"
                                "http://cdn.example.com/hls/seg-006.ts\n"
                                "#EXTINF:4.6,segment 7\n"
                                "720p/seg-007.ts\n"
                                "#EXTINF:4.7,segment 8\n"
                                "audio/seg-008.aac\n"
                                "#EXTINF:4.8,segment 9\n"
                                "subtitles/seg-009.vtt\n"
                                "#EXTINF:4.9,segment 10\n"
                                "seg-010.ts?token=abc\n";

    hlsapi::HLSParser parser;
    parser.setOriginUri("https://stream.example.com/live/master/index.m3u8");
    ASSERT_TRUE(parser.parse(playlist));

    const auto& parsed = parser.playlist();
    ASSERT_EQ(parsed.segments.size(), 10);
    EXPECT_FALSE(parser.isMaster());
    EXPECT_DOUBLE_EQ(parsed.durationAll(), 44.5);
    EXPECT_EQ(parsed.keyInfo.method, hlsapi::KeyInfo::Aes128);
    EXPECT_EQ(parsed.keyInfo.getUri(), "https://stream.example.com/live/master/keys/key.bin");

    for (std::size_t index = 0; index < kExpectedSegmentUris.size(); ++index)
    {
        EXPECT_EQ(parsed.segments[index].getUri(), kExpectedSegmentUris[index]) << index;
    }
}

TEST(HLSParserUnitTest, ParsesMasterPlaylistVariants)
{
    constexpr char playlist[] = "#EXTM3U\n"
                                "#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID=\"audio\",NAME=\"main\",DEFAULT=YES,AUTOSELECT=YES,URI=\"audio/main.m3u8\"\n"
                                "#EXT-X-STREAM-INF:BANDWIDTH=800000,RESOLUTION=640x360,CODECS=\"avc1.42e01e,mp4a.40.2\",AUDIO=\"audio\"\n"
                                "360p/index.m3u8\n"
                                "#EXT-X-STREAM-INF:BANDWIDTH=1400000,RESOLUTION=1280x720,CODECS=\"avc1.4d401f,mp4a.40.2\",AUDIO=\"audio\"\n"
                                "720p/index.m3u8\n";

    hlsapi::HLSParser parser;
    parser.setOriginUri("https://stream.example.com/live/master.m3u8");
    ASSERT_TRUE(parser.parse(playlist));

    EXPECT_TRUE(parser.isMaster());
    ASSERT_EQ(parser.master().streams.size(), 2);
    ASSERT_EQ(parser.master().mediaInfos.size(), 1);
    EXPECT_EQ(parser.master().streams[0].getUri(), "https://stream.example.com/live/360p/index.m3u8");
    EXPECT_EQ(parser.master().mediaInfos[0].getUri(), "https://stream.example.com/live/audio/main.m3u8");
}

TEST(HLSApiUnitTest, ParsesQuotedAttributesAndBooleanForms)
{
    const auto attributes = hlsapi::parseAttributes(R"(TYPE=AUDIO,GROUP-ID="stereo,audio",DEFAULT=YES,AUTOSELECT=1,NAME="Main")");

    EXPECT_EQ(attributes.at("TYPE"), "AUDIO");
    EXPECT_EQ(attributes.at("GROUP-ID"), "stereo,audio");
    EXPECT_EQ(attributes.at("NAME"), "Main");
    EXPECT_TRUE(hlsapi::attrBool(attributes.at("DEFAULT")));
    EXPECT_TRUE(hlsapi::attrBool(attributes.at("AUTOSELECT")));
    EXPECT_FALSE(hlsapi::attrBool("NO"));
    EXPECT_EQ(hlsapi::trim(" \t playlist value\r\n"), "playlist value");
}

TEST(HLSApiUnitTest, ParsesMediaKeyMapAndSegmentTags)
{
    hlsapi::MediaInfo media;
    ASSERT_TRUE(
        media.parseContent(R"(#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="audio",LANGUAGE="zh",NAME="Chinese",DEFAULT=YES,AUTOSELECT=YES,URI="audio/index.m3u8")"));
    EXPECT_EQ(media.type, hlsapi::MediaInfo::Audio);
    EXPECT_EQ(media.groupId, "audio");
    EXPECT_EQ(media.language, "zh");
    EXPECT_TRUE(media.strDefault);
    EXPECT_TRUE(media.autoSelect);
    EXPECT_EQ(media.uri, "audio/index.m3u8");

    hlsapi::KeyInfo key;
    ASSERT_TRUE(key.parseContent(R"(#EXT-X-KEY:METHOD=AES-128,URI="key.bin",IV=0x01,KEYFORMAT="identity")"));
    EXPECT_EQ(key.method, hlsapi::KeyInfo::Aes128);
    EXPECT_EQ(key.uri, "key.bin");
    EXPECT_EQ(key.iv, "0x01");
    EXPECT_EQ(key.keyFormat, "identity");

    hlsapi::MediaMap map;
    ASSERT_TRUE(map.parseContent(R"(#EXT-X-MAP:URI="init.mp4",BYTERANGE="1024@256")"));
    EXPECT_EQ(map.uri, "init.mp4");
    EXPECT_EQ(map.byteRangeLength, 1024);
    EXPECT_EQ(map.byteRangeOffset, 256);

    hlsapi::MediaSegment segment;
    ASSERT_TRUE(segment.parseContent("#EXTINF:6.25,chapter one"));
    EXPECT_DOUBLE_EQ(segment.duration, 6.25);
    EXPECT_EQ(segment.title, "chapter one");
}

TEST(HLSApiUnitTest, ResolvesAbsoluteRootAndNestedRelativeUris)
{
    hlsapi::PendingInfo info;
    info.baseUri = "https://media.example.com/a/b/c/";

    info.uri = "segment.ts";
    EXPECT_EQ(info.getUri(), "https://media.example.com/a/b/c/segment.ts");
    info.uri = "../../shared/segment.ts";
    EXPECT_EQ(info.getUri(), "https://media.example.com/a/shared/segment.ts");
    info.uri = "/root/segment.ts";
    EXPECT_EQ(info.getUri(), "https://media.example.com/root/segment.ts");
    info.uri = "https://cdn.example.com/segment.ts";
    EXPECT_EQ(info.getUri(), "https://cdn.example.com/segment.ts");
}

TEST(HLSParserUnitTest, ParsesCrLfPlaylistWithoutEmptySegments)
{
    hlsapi::HLSParser parser;
    parser.setOriginUri("https://example.invalid/live/index.m3u8");
    ASSERT_TRUE(parser.parse("#EXTM3U\r\n#EXTINF:1.5,clip\r\nsegment.ts\r\n"));
    ASSERT_EQ(parser.playlist().segments.size(), 1);
    EXPECT_DOUBLE_EQ(parser.playlist().segments[0].duration, 1.5);
    EXPECT_EQ(parser.playlist().segments[0].getUri(), "https://example.invalid/live/segment.ts");
}
