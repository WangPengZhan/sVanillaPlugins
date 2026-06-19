#include <array>

#include <gtest/gtest.h>

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
