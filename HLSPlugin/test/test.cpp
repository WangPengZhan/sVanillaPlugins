#include <string>

#include <gtest/gtest.h>

#include "IPlugin.h"
#include "TemplatePluginCall.h"

TEST(HLSPlugin_Test, test)
{
    std::cout << "test";
}

TEST(HLSPlugin_Test, INFO)
{
    initDir("D:/");
    auto hanlde = pluginInit();
    auto plugin = reinterpret_cast<plugin::IPlugin*>(hanlde);
    auto info = plugin->pluginMessage();
    EXPECT_EQ(info.name, "template");
    EXPECT_EQ(info.pluginId, 0);
    EXPECT_EQ(info.domain, "https://sVanilla.app");

    auto result = plugin->canParseUrl("https://bitdash-a.akamaihd.net/content/sintel/hls/playlist.m3u8");
    EXPECT_TRUE(result);
    auto videoView = plugin->getVideoView("https://bitdash-a.akamaihd.net/content/sintel/hls/playlist.m3u8");
    EXPECT_FALSE(videoView.empty());

    VideoInfoFull videoInfo;
    videoInfo.videoView = std::make_shared<adapter::BaseVideoView>(videoView[0]);
    videoInfo.downloadConfig = std::make_shared<DownloadConfig>();
    videoInfo.downloadConfig->downloadDir = "D:/test/";
    videoInfo.downloadConfig->nameRule = "test.mp4";
    auto downloader = plugin->getDownloader(videoInfo);

    EXPECT_TRUE(downloader != nullptr);

    downloader->start();

    std::this_thread::sleep_for(std::chrono::seconds(200));
}