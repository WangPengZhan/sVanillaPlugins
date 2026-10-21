#include <array>
#include <chrono>
#include <filesystem>

#include <gtest/gtest.h>
#include <openssl/evp.h>

#include "LoggerRegisterHelpper.h"
#include "HLSApi/HLSApi.h"
#include "HLSApi/HLSClient.h"
#include "HLSApi/HLSLog.h"
#include "HLSApi/HLSParser.h"
#include "HLSApi/HLSPlaylistDownloader.h"

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

class HLSLoggerEnvironment final : public testing::Environment
{
public:
    void SetUp() override
    {
        m_logDir = std::filesystem::temp_directory_path() /
                   ("hls-unit-test-log-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(m_logDir);
        LoggerRegisterHelpper::registerLogger(HLSModuleName, (m_logDir / "HLS.log").string());
    }

    void TearDown() override
    {
        LoggerRegisterHelpper::unregisterLogger(HLSModuleName);
        std::filesystem::remove_all(m_logDir);
    }

private:
    std::filesystem::path m_logDir;
};

testing::Environment* const kHLSLoggerEnvironment = testing::AddGlobalTestEnvironment(new HLSLoggerEnvironment);

std::vector<std::uint8_t> encryptAes(const std::string& plaintext, const std::string& key, const std::array<unsigned char, 16>& iv)
{
    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    std::vector<std::uint8_t> output(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
    int firstLength = 0;
    int finalLength = 0;
    EXPECT_EQ(EVP_EncryptInit_ex(context, EVP_aes_128_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key.data()), iv.data()), 1);
    EXPECT_EQ(
        EVP_EncryptUpdate(context, output.data(), &firstLength, reinterpret_cast<const unsigned char*>(plaintext.data()), static_cast<int>(plaintext.size())),
        1);
    EXPECT_EQ(EVP_EncryptFinal_ex(context, output.data() + firstLength, &finalLength), 1);
    EVP_CIPHER_CTX_free(context);
    output.resize(firstLength + finalLength);
    return output;
}

}  // namespace

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
    EXPECT_TRUE(map.hasByteRange);
    EXPECT_TRUE(map.hasByteRangeOffset);

    hlsapi::MediaMap implicitOffsetMap;
    ASSERT_TRUE(implicitOffsetMap.parseContent(R"(#EXT-X-MAP:URI="init.mp4",BYTERANGE="512")"));
    EXPECT_TRUE(implicitOffsetMap.hasByteRange);
    EXPECT_FALSE(implicitOffsetMap.hasByteRangeOffset);

    hlsapi::MediaSegment segment;
    ASSERT_TRUE(segment.parseContent("#EXTINF:6.25,chapter one"));
    EXPECT_DOUBLE_EQ(segment.duration, 6.25);
    EXPECT_EQ(segment.title, "chapter one");
    EXPECT_FALSE(segment.parseContent("#EXTINF:6.25"));
    EXPECT_FALSE(segment.parseContent("#EXTINF,chapter one"));
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

TEST(HLSApiRegression, BoundedAttributesAndFractionalFrameRate)
{
    EXPECT_TRUE(hlsapi::parseAttributes("X=").empty());
    EXPECT_TRUE(hlsapi::parseAttributes("X=1,X=2").empty());
    EXPECT_TRUE(hlsapi::parseAttributes("X=1,").empty());
    EXPECT_TRUE(hlsapi::parseAttributes("X=1,broken").empty());
    EXPECT_TRUE(hlsapi::parseAttributes("X=1,Y=\"unterminated").empty());
    EXPECT_EQ(hlsapi::parseAttributes(" X = 1 , Y = \"a,b\" ").at("Y"), "a,b");

    hlsapi::StreamInfo stream;
    ASSERT_TRUE(stream.parseContent("#EXT-X-STREAM-INF:BANDWIDTH=4294967296,FRAME-RATE=29.97,RESOLUTION=1920x1080"));
    EXPECT_EQ(stream.bandwidth, 4294967296ULL);
    EXPECT_DOUBLE_EQ(stream.frameRate, 29.97);
}

TEST(HLSParserRegression, PreservesUnparsedTagsAndUnknownKeyMethods)
{
    hlsapi::HLSParser parser;
    parser.setOriginUri("https://example.invalid/index.m3u8");
    ASSERT_TRUE(parser.parse("#EXTM3U\n#EXT-X-VERSION:7\n#EXT-X-PLAYLIST-TYPE:VOD\n"
                             "#EXT-X-KEY:METHOD=FUTURE-METHOD,URI=\"key.bin\"\n#EXTINF:1,clip\nseg.ts\n"));
    EXPECT_EQ(parser.unparsedTags(), (std::vector<std::string>{"#EXT-X-VERSION:7", "#EXT-X-PLAYLIST-TYPE:VOD"}));
    ASSERT_EQ(parser.playlist().segments.size(), 1);
    EXPECT_EQ(parser.playlist().segments[0].keyInfo.method, hlsapi::KeyInfo::None);
    EXPECT_EQ(parser.playlist().segments[0].keyInfo.strMethod, "FUTURE-METHOD");
    EXPECT_FALSE(parser.playlist().isSupportedVod());
}

TEST(HLSParserRegression, ParsesAndResolvesSegmentByteRanges)
{
    hlsapi::HLSParser parser;
    parser.setOriginUri("https://example.invalid/vod/index.m3u8");
    ASSERT_TRUE(parser.parse("#EXTM3U\n#EXT-X-TARGETDURATION:2\n#EXT-X-MEDIA-SEQUENCE:4\n"
                             "#EXTINF:1,one\n#EXT-X-BYTERANGE:100@20\nmedia.mp4\n"
                             "#EXTINF:1,two\n#EXT-X-BYTERANGE:80\nmedia.mp4\n#EXT-X-ENDLIST\n"));
    const auto& segments = parser.playlist().segments;
    ASSERT_EQ(segments.size(), 2);
    EXPECT_EQ(segments[0].byteRangeLength, 100);
    EXPECT_EQ(segments[0].byteRangeOffset, 20);
    EXPECT_TRUE(segments[0].hasByteRangeOffset);
    EXPECT_EQ(segments[1].byteRangeLength, 80);
    EXPECT_EQ(segments[1].byteRangeOffset, 120);
    EXPECT_TRUE(segments[1].hasByteRangeOffset);
    EXPECT_TRUE(parser.playlist().isSupportedVod());
    EXPECT_FALSE(parser.parse("#EXTM3U\n#EXTINF:1,one\n#EXT-X-BYTERANGE:100\nmedia.mp4\n"));
}

TEST(HLSParserRegression, RejectsMalformedInputWithoutPartialResults)
{
    hlsapi::HLSParser parser;
    parser.setOriginUri("https://example.invalid/index.m3u8");
    ASSERT_TRUE(parser.parse("#EXTM3U\n#EXTINF:1,ok\na.ts\n#EXT-X-ENDLIST\n"));
    ASSERT_EQ(parser.playlist().segments.size(), 1);
    EXPECT_FALSE(parser.parse("<html>404</html>"));
    EXPECT_TRUE(parser.playlist().segments.empty());
    EXPECT_FALSE(parser.parse("#EXTM3U\n#EXTINF:1,missing-uri\n"));
    EXPECT_FALSE(parser.parse("#EXTM3U\n#EXTINF:1,missing-uri\n#EXTINF:1,next\nb.ts\n"));
    EXPECT_FALSE(parser.parse("#EXTM3U\n#EXTINF:1,invalid-uri\nfile:///tmp/a.ts\n"));
    EXPECT_FALSE(parser.parse("#EXTM3U\n#EXT-X-TARGETDURATION:0\n#EXTINF:1,a\na.ts\n"));
    EXPECT_FALSE(parser.parse("#EXTM3U\n#EXTINF:1,a\na.ts\n#EXT-X-MEDIA-SEQUENCE:5\n"));
    EXPECT_FALSE(parser.parse("#EXTM3U\n#EXT-X-MEDIA-SEQUENCE:18446744073709551615\n#EXTINF:1,a\na.ts\n#EXTINF:1,b\nb.ts\n"));
}

TEST(HLSParserRegression, SnapshotsRotatingKeysAndMediaSequence)
{
    constexpr char playlist[] = "#EXTM3U\n"
                                "#EXT-X-MEDIA-SEQUENCE:42\n"
                                "#EXT-X-TARGETDURATION:8\n"
                                "#EXT-X-KEY:METHOD=AES-128,URI=key1.bin\n"
                                "#EXTINF:1,one\n1.ts\n"
                                "#EXT-X-KEY:METHOD=AES-128,URI=key2.bin\n"
                                "#EXTINF:1,two\n2.ts\n"
                                "#EXT-X-KEY:METHOD=NONE\n"
                                "#EXTINF:1,three\n3.ts\n"
                                "#EXT-X-ENDLIST\n";
    hlsapi::HLSParser parser;
    parser.setOriginUri("https://example.invalid/live/index.m3u8");
    ASSERT_TRUE(parser.parse(playlist));
    const auto& parsed = parser.playlist();
    ASSERT_EQ(parsed.segments.size(), 3);
    EXPECT_EQ(parsed.mediaSequence, 42);
    EXPECT_DOUBLE_EQ(parsed.targetDuration, 8);
    EXPECT_EQ(parsed.segments[0].mediaSequence, 42);
    EXPECT_EQ(parsed.segments[1].mediaSequence, 43);
    EXPECT_EQ(parsed.segments[0].keyInfo.getUri(), "https://example.invalid/live/key1.bin");
    EXPECT_EQ(parsed.segments[1].keyInfo.getUri(), "https://example.invalid/live/key2.bin");
    EXPECT_EQ(parsed.segments[2].keyInfo.method, hlsapi::KeyInfo::None);
    EXPECT_TRUE(parsed.isSupportedVod());
}

TEST(HLSParserRegression, SupportsInitializationMapButRejectsOtherUnsupportedFeatures)
{
    hlsapi::HLSParser parser;
    parser.setOriginUri("https://example.invalid/index.m3u8");
    ASSERT_TRUE(parser.parse("#EXTM3U\n#EXT-X-MAP:URI=\"init.mp4\"\n#EXTINF:1,clip\na.m4s\n#EXT-X-ENDLIST\n"));
    EXPECT_TRUE(parser.playlist().hasMap);
    EXPECT_TRUE(parser.playlist().isSupportedVod());
}

TEST(HLSClientRegression, DecryptsExplicitAndSequenceDerivedIv)
{
    std::string key;
    for (int value = 0; value < 16; ++value)
    {
        key.push_back(static_cast<char>(value));
    }
    std::array<unsigned char, 16> iv{};
    iv[14] = 1;
    iv[15] = 2;
    const std::string plaintext = "HLS regression payload";
    const auto cipher = encryptAes(plaintext, key, iv);

    hlsapi::KeyInfo keyInfo;
    keyInfo.method = hlsapi::KeyInfo::Aes128;
    keyInfo.key = key;
    std::vector<std::uint8_t> decrypted;
    ASSERT_TRUE(hlsapi::decryptSegment(cipher, keyInfo, 258, decrypted));
    EXPECT_EQ(std::string(decrypted.begin(), decrypted.end()), plaintext);
    keyInfo.iv = "0x0102";
    ASSERT_TRUE(hlsapi::decryptSegment(cipher, keyInfo, 0, decrypted));
    EXPECT_EQ(std::string(decrypted.begin(), decrypted.end()), plaintext);
    keyInfo.key = "short";
    EXPECT_FALSE(hlsapi::decryptSegment(cipher, keyInfo, 258, decrypted));
}

TEST(HLSClientRegression, RejectsEmptyOutputFilename)
{
    hlsapi::HLSClient client;
    EXPECT_FALSE(client.downloadTsToFiles("https://example.invalid/segment.ts", ""));
}

TEST(HLSQueueRegression, RejectsEntireBatchBeforeEnqueuing)
{
    hlsapi::HLSClient client;
    const auto directory =
        std::filesystem::temp_directory_path() / ("hls-queue-batch-regression-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    hlsapi::HLSPlaylistDownloader downloader(client, directory.string());
    hlsapi::M3U8Playlist supported;
    supported.hasEndList = true;
    hlsapi::MediaSegment segment;
    segment.uri = "https://example.invalid/segment.ts";
    supported.segments.push_back(segment);
    auto unsupported = supported;
    unsupported.hasMap = true;

    EXPECT_FALSE(downloader.downloadTsToFiles({supported, unsupported}));
    EXPECT_EQ(downloader.totalCount(), 0);
    EXPECT_FALSE(downloader.downloadTsToFiles({}));
    EXPECT_TRUE(downloader.downloadTsToFiles({supported}));
    EXPECT_EQ(downloader.totalCount(), 1);
    downloader.exitDownload();
    std::filesystem::remove_all(directory);
}

TEST(HLSMapRegression, ParsesAndQueuesInitializationBeforeFragments)
{
    hlsapi::HLSParser parser;
    parser.setOriginUri("https://example.invalid/vod/index.m3u8");
    ASSERT_TRUE(parser.parse("#EXTM3U\n#EXT-X-TARGETDURATION:2\n#EXT-X-MAP:URI=\"init.mp4\"\n#EXTINF:1,one\n1.m4s\n#EXTINF:1,two\n2.m4s\n#EXT-X-ENDLIST\n"));
    const auto& playlist = parser.playlist();
    ASSERT_TRUE(playlist.hasMap);
    EXPECT_EQ(playlist.mediaMap.getUri(), "https://example.invalid/vod/init.mp4");
    EXPECT_TRUE(playlist.isSupportedVod());

    hlsapi::HLSClient client;
    const auto directory =
        std::filesystem::temp_directory_path() / ("hls-map-regression-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    hlsapi::HLSPlaylistDownloader downloader(client, directory.string());
    ASSERT_TRUE(downloader.downloadTsToFiles({playlist}));
    EXPECT_EQ(downloader.totalCount(), 3);
    downloader.exitDownload();
    std::filesystem::remove_all(directory);
}

TEST(HLSMapRegression, QueuesMapRotationsAndByteRanges)
{
    hlsapi::HLSParser parser;
    parser.setOriginUri("https://example.invalid/vod/index.m3u8");
    ASSERT_TRUE(parser.parse("#EXTM3U\n#EXT-X-TARGETDURATION:2\n"
                             "#EXT-X-MAP:URI=\"media.mp4\",BYTERANGE=\"16@0\"\n"
                             "#EXTINF:1,one\n#EXT-X-BYTERANGE:100@16\nmedia.mp4\n"
                             "#EXT-X-MAP:URI=\"media.mp4\",BYTERANGE=\"12@116\"\n"
                             "#EXTINF:1,two\n#EXT-X-BYTERANGE:80@128\nmedia.mp4\n#EXT-X-ENDLIST\n"));
    const auto& playlist = parser.playlist();
    ASSERT_EQ(playlist.mediaMaps.size(), 2);
    ASSERT_EQ(playlist.segments.size(), 2);
    ASSERT_TRUE(playlist.isSupportedVod());

    hlsapi::HLSClient client;
    const auto directory = std::filesystem::temp_directory_path() /
                           ("hls-map-range-regression-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    hlsapi::HLSPlaylistDownloader downloader(client, directory.string());
    ASSERT_TRUE(downloader.downloadTsToFiles({playlist}));
    EXPECT_EQ(downloader.totalCount(), 4);
    downloader.exitDownload();
    std::filesystem::remove_all(directory);
}

TEST(HLSMapRegression, ParsesProtocolMapsAndLeavesDownloadSupportDecisionSeparate)
{
    hlsapi::HLSParser parser;
    parser.setOriginUri("https://example.invalid/vod/index.m3u8");
    const std::string tail = "#EXTINF:1,one\n1.m4s\n#EXT-X-ENDLIST\n";
    EXPECT_FALSE(parser.parse("#EXTM3U\n#EXT-X-MAP:BYTERANGE=100@0\n" + tail));
    EXPECT_FALSE(parser.parse("#EXTM3U\n#EXT-X-MAP\n" + tail));

    ASSERT_TRUE(parser.parse("#EXTM3U\n#EXT-X-MAP:URI=\"init.mp4\",BYTERANGE=\"1024@32\"\n" + tail));
    const auto& ranged = parser.playlist();
    ASSERT_EQ(ranged.mediaMaps.size(), 1);
    EXPECT_TRUE(ranged.mediaMaps[0].hasByteRange);
    EXPECT_EQ(ranged.mediaMaps[0].byteRangeLength, 1024);
    EXPECT_EQ(ranged.mediaMaps[0].byteRangeOffset, 32);
    EXPECT_TRUE(ranged.mediaMaps[0].hasByteRangeOffset);
    ASSERT_EQ(ranged.segments.size(), 1);
    ASSERT_TRUE(ranged.segments[0].mediaMapIndex.has_value());
    EXPECT_EQ(*ranged.segments[0].mediaMapIndex, 0);
    EXPECT_TRUE(ranged.isSupportedVod());

    ASSERT_TRUE(parser.parse("#EXTM3U\n#EXT-X-MAP:URI=\"first.mp4\"\n#EXTINF:1,one\n1.m4s\n"
                             "#EXTINF:1,two\n#EXT-X-MAP:URI=\"second.mp4\"\n2.m4s\n#EXT-X-ENDLIST\n"));
    const auto& rotated = parser.playlist();
    ASSERT_EQ(rotated.mediaMaps.size(), 2);
    EXPECT_EQ(rotated.mediaMaps[0].uri, "first.mp4");
    EXPECT_EQ(rotated.mediaMaps[1].uri, "second.mp4");
    ASSERT_EQ(rotated.segments.size(), 2);
    EXPECT_EQ(rotated.segments[0].mediaMapIndex, 0);
    EXPECT_EQ(rotated.segments[1].mediaMapIndex, 1);
    EXPECT_TRUE(rotated.isSupportedVod());

    ASSERT_TRUE(parser.parse("#EXTM3U\n#EXT-X-KEY:METHOD=AES-128,URI=\"key.bin\",IV=0x01\n"
                             "#EXT-X-MAP:URI=\"encrypted-init.mp4\"\n" +
                             tail));
    ASSERT_EQ(parser.playlist().mediaMaps.size(), 1);
    EXPECT_EQ(parser.playlist().mediaMaps[0].keyInfo.method, hlsapi::KeyInfo::Aes128);
    EXPECT_EQ(parser.playlist().mediaMaps[0].keyInfo.iv, "0x01");
    EXPECT_TRUE(parser.playlist().isSupportedVod());

    EXPECT_FALSE(parser.parse("#EXTM3U\n#EXT-X-KEY:METHOD=AES-128,URI=\"key.bin\"\n"
                              "#EXT-X-MAP:URI=\"encrypted-init.mp4\"\n" +
                              tail));
}
