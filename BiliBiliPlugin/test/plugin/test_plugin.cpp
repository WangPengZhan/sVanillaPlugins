#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "IPlugin.h"
#include "Plugin/BiliBiliPlugin.h"
#include "TemplatePluginCall.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <filesystem>
#endif

namespace
{
constexpr std::array<const char*, 10> kSupportedUrls = {
    "https://www.bilibili.com/video/av170001",
    "https://www.bilibili.com/video/BV1xx411c7mD",
    "https://b23.tv/BV1xx411c7mD",
    "https://www.bilibili.com/bangumi/play/ss90001",
    "https://www.bilibili.com/bangumi/play/ep90002",
    "https://www.bilibili.com/bangumi/media/md90003",
    "https://www.bilibili.com/cheese/play/ss80001",
    "https://www.bilibili.com/cheese/play/ep80002",
    "https://www.bilibili.com/medialist/detail/ml70001",
    "https://space.bilibili.com/60001",
};

constexpr auto kDownloadTimeout = std::chrono::minutes(10);
constexpr auto kDownloadPollInterval = std::chrono::seconds(1);
const std::string kRuntimeDir = "bili-plugin-test/runtime";
const std::string kDownloadDir = "bili-plugin-test/download";

struct PluginDeinitGuard
{
    ~PluginDeinitGuard()
    {
        pluginDeinit();
    }
};

void disableFileLoggerForTest()
{
#ifdef _WIN32
    SetEnvironmentVariableA("SVANILLA_PLUGIN_TEST_NO_FILE_LOG", "1");
#else
    setenv("SVANILLA_PLUGIN_TEST_NO_FILE_LOG", "1", 1);
#endif
}

void createDirectoryIfMissing(const std::string& path)
{
#ifdef _WIN32
    CreateDirectoryA(path.c_str(), nullptr);
#else
    std::filesystem::create_directories(path);
#endif
}

void prepareRuntimeDirectories()
{
    createDirectoryIfMissing("bili-plugin-test");
    createDirectoryIfMissing(kRuntimeDir);
    createDirectoryIfMissing(kRuntimeDir + "/log");
    createDirectoryIfMissing(kDownloadDir);
}

const char* statusText(download::AbstractDownloader::Status status)
{
    switch (status)
    {
    case download::AbstractDownloader::Waiting:
        return "Waiting";
    case download::AbstractDownloader::Ready:
        return "Ready";
    case download::AbstractDownloader::Downloading:
        return "Downloading";
    case download::AbstractDownloader::Pause:
        return "Pause";
    case download::AbstractDownloader::Paused:
        return "Paused";
    case download::AbstractDownloader::Resumed:
        return "Resumed";
    case download::AbstractDownloader::Stopped:
        return "Stopped";
    case download::AbstractDownloader::Finished:
        return "Finished";
    case download::AbstractDownloader::Error:
        return "Error";
    }

    return "Unknown";
}

void printView(const adapter::BaseVideoView& view, std::size_t index)
{
    std::cout << "Bili view[" << index << "]"
              << " Identifier=" << view.Identifier
              << " IdType=" << view.IdType
              << " Title=" << view.Title
              << " Publisher=" << view.Publisher
              << " Duration=" << view.Duration
              << " PublishDate=" << view.PublishDate
              << " Option1=" << view.Option1
              << " Option2=" << view.Option2
              << " Extension=" << view.fileExtension
              << " PluginId=" << view.pluginId << std::endl;
}

void expectPrimaryView(const adapter::BaseVideoView& view)
{
    std::cout << "Bili primary view compare:"
              << " expected Identifier=BV1xx411c7mD actual=" << view.Identifier
              << ", expected IdType=Bid actual=" << view.IdType
              << ", expected PluginId=1 actual=" << view.pluginId
              << ", expected Extension=.mp4 actual=" << view.fileExtension << std::endl;

    EXPECT_EQ(view.Identifier, "BV1xx411c7mD");
    EXPECT_EQ(view.IdType, "Bid");
    EXPECT_EQ(view.pluginId, 1);
    EXPECT_EQ(view.fileType, adapter::FileType::Video);
    EXPECT_EQ(view.fileExtension, ".mp4");
    EXPECT_FALSE(view.Title.empty());
    EXPECT_FALSE(view.Publisher.empty());
    EXPECT_FALSE(view.Duration.empty());
    EXPECT_FALSE(view.Option1.empty());
    EXPECT_FALSE(view.Option2.empty());
}
}  // namespace

TEST(BiliBiliPluginArtifactTest, BusinessFlowThroughPluginInterfaces)
{
    std::cout << "Bili artifact test: prepare runtime directories" << std::endl;
    prepareRuntimeDirectories();
    std::cout << "Bili artifact test: runtime directories ready" << std::endl;
    disableFileLoggerForTest();
    std::cout << "Bili artifact test: file logger disabled" << std::endl;
    initDir((kRuntimeDir + "/").c_str());
    std::cout << "Bili artifact test: initDir done" << std::endl;

    std::cout << "Bili artifact test: pluginInit" << std::endl;
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);
    const PluginDeinitGuard deinit;

    std::cout << "Bili artifact test: check plugin metadata" << std::endl;
    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);
    const auto& info = plugin->pluginMessage();
    EXPECT_EQ(info.name, "BiliBili");
    EXPECT_EQ(info.pluginId, 1);
    EXPECT_EQ(info.domain, "https://www.bilibili.com");
    EXPECT_FALSE(plugin->websiteIcon().empty());

    std::cout << "Bili artifact test: check supported urls" << std::endl;
    for (const auto* url : kSupportedUrls)
    {
        EXPECT_TRUE(plugin->canParseUrl(url)) << url;
    }

    std::cout << "Bili artifact test: getVideoView" << std::endl;
    const auto views = plugin->getVideoView("https://www.bilibili.com/video/BV1xx411c7mD");
    std::cout << "Bili artifact test: getVideoView returned " << views.size() << " views" << std::endl;
    if (views.empty())
    {
        GTEST_SKIP() << "Bili video view lookup is network dependent in this environment";
    }
    ASSERT_FALSE(views.empty());
    for (std::size_t i = 0; i < views.size(); ++i)
    {
        printView(views[i], i);
    }
    expectPrimaryView(views.front());

    VideoInfoFull videoInfo;
    videoInfo.downloadConfig = std::make_shared<DownloadConfig>();
    videoInfo.downloadConfig->downloadDir = kDownloadDir + "/";
    videoInfo.videoView = std::make_shared<adapter::BaseVideoView>(views.front());

    std::cout << "Bili artifact test: getDownloader" << std::endl;
    auto downloader = plugin->getDownloader(videoInfo);
    if (!downloader)
    {
        GTEST_SKIP() << "Bili downloader creation is network dependent in this environment";
    }

    std::cout << "Bili downloader created:"
              << " path=" << downloader->path()
              << " filename=" << downloader->filename()
              << " status=" << statusText(downloader->status()) << std::endl;

    downloader->start();
    ASSERT_NE(downloader->status(), download::AbstractDownloader::Error) << "Bili downloader failed to start";

    const auto deadline = std::chrono::steady_clock::now() + kDownloadTimeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        downloader->downloadStatus();
        const auto& downloadInfo = downloader->info();
        std::cout << "Bili downloader progress:"
                  << " status=" << statusText(downloader->status())
                  << " complete=" << downloadInfo.complete
                  << " total=" << downloadInfo.total
                  << " speed=" << downloadInfo.speed
                  << " stage=" << downloadInfo.stage << std::endl;

        if (downloader->status() == download::AbstractDownloader::Finished)
        {
            break;
        }

        ASSERT_NE(downloader->status(), download::AbstractDownloader::Error) << "Bili downloader reported Error";
        std::this_thread::sleep_for(kDownloadPollInterval);
    }

    EXPECT_EQ(downloader->status(), download::AbstractDownloader::Finished) << "Bili downloader did not finish before timeout";
}
