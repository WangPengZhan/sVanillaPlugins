#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <PluginCommon/BaseVideoView.h>

#include "Plugin/YoutubePlugin.h"
#include "TemplatePluginCall.h"

namespace
{
void formatPluginMessage(const PluginMessage& message)
{
    std::cout << "Plugin ID: " << message.pluginId << std::endl;
    std::cout << "Name: " << message.name << std::endl;
    std::cout << "Version: " << message.version << std::endl;
    std::cout << "Description: " << message.description << std::endl;
    std::cout << "Domain: " << message.domain << std::endl;
}

void formatVideoView(const adapter::VideoView& views)
{
    std::cout << "views size: " << views.size() << std::endl;
    for (const auto& view : views)
    {
        std::cout << "Video ID: " << view.Identifier << std::endl;
        std::cout << "Title: " << view.Title << std::endl;
        std::cout << "Publisher: " << view.Publisher << std::endl;
        std::cout << "Cover: " << view.Cover << std::endl;
        std::cout << "Duration: " << view.Duration << std::endl;
        std::cout << "Description: " << view.Description << std::endl;
        std::cout << "Publish Date: " << view.PublishDate << std::endl;
        std::cout << "Playlist Title: " << view.PlayListTitle << std::endl;
        std::cout << "File Type: " << static_cast<int>(view.fileType) << std::endl;
    }
}
}  // namespace

TEST(YoutubPlugin_Test, PluginInfo)
{
    initDir("");
    auto handle = pluginInit();
    auto plugin = reinterpret_cast<YoutubePlugin*>(handle);
    auto info = plugin->pluginMessage();
    formatPluginMessage(info);
    std::cout << "test";
}

TEST(YoutubPlugin_Test, VideoInfo)
{
    auto handle = pluginInit();
    auto plugin = reinterpret_cast<YoutubePlugin*>(handle);
    auto views = plugin->getVideoView("https://www.youtube.com/watch?v=q-F7M2PmgeI");
    ASSERT_FALSE(views.empty()) << "Video view should not be empty";
    formatVideoView(views);
}

TEST(YoutubPlugin_Test, downloader)
{
    auto handle = pluginInit();
    auto plugin = reinterpret_cast<YoutubePlugin*>(handle);
    auto views = plugin->getVideoView("https://www.youtube.com/watch?v=q-F7M2PmgeI");
    ASSERT_FALSE(views.empty()) << "Video view should not be empty";
    VideoInfoFull videoInfo;
    videoInfo.videoView = std::make_shared<adapter::BaseVideoView>(views.front());
    videoInfo.downloadConfig = std::make_shared<DownloadConfig>();
    auto downloader = plugin->getDownloader(videoInfo);
    downloader->start();
    ASSERT_FALSE(downloader) << "Video view should not be empty";
    formatVideoView(views);
}
