#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "IPlugin.h"
#include "TemplatePluginCall.h"
#include "LoggerRegisterHelpper.h"
#include "Util/process.hpp"
#include "Util/TimerUtil.h"
#include "Util/LocaleHelper.h"

namespace
{
using json = nlohmann::json;

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

constexpr auto kDownloadPollInterval = std::chrono::seconds(1);
const std::filesystem::path kRuntimeDir = "bili-plugin-test/runtime";
const std::filesystem::path kCaseFile = "business_flow_cases.json";

struct PluginDeinitGuard
{
    ~PluginDeinitGuard()
    {
        pluginDeinit();
        deinit();
    }
};

std::filesystem::path ariaExecutableName()
{
#ifdef _WIN32
    return "aria2c.exe";
#else
    return "aria2c";
#endif
}

TinyProcessLib::Process::string_type PathToProcessString(const std::filesystem::path& path)
{
#if defined(_WIN32) && defined(UNICODE)
    return path.wstring();
#else
    return path.string();
#endif
}

TinyProcessLib::Process::string_type toProcessString(const std::string& value)
{
#if defined(_WIN32) && defined(UNICODE)
    return std::filesystem::path(value).wstring();
#else
    return value;
#endif
}

std::vector<TinyProcessLib::Process::string_type> ariaArguments(const std::filesystem::path& runtimeDir)
{
    const auto ariaDir = runtimeDir / "aria";
    std::filesystem::create_directories(ariaDir);
    const auto sessionFile = ariaDir / "aira.session";
    const auto logFile = ariaDir / "aira.log";
    std::ofstream(sessionFile, std::ios::app).close();

    return {
        toProcessString("--enable-rpc"),
        toProcessString("--rpc-listen-all=false"),
        toProcessString("--rpc-allow-origin-all=false"),
        toProcessString("--rpc-listen-port=6800"),
        toProcessString("--rpc-secret=sVanilla"),
        toProcessString("--input-file=" + sessionFile.string()),
        toProcessString("--save-session=" + sessionFile.string()),
        toProcessString("--save-session-interval=30"),
        toProcessString("--log=" + logFile.string()),
        toProcessString("--log-level=debug"),
        toProcessString("--max-concurrent-downloads=6"),
        toProcessString("--max-connection-per-server=16"),
        toProcessString("--split=5"),
        toProcessString("--min-split-size=10M"),
        toProcessString("--max-overall-download-limit=0"),
        toProcessString("--max-download-limit=0"),
        toProcessString("--max-overall-upload-limit=0"),
        toProcessString("--max-upload-limit=0"),
        toProcessString("--continue=true"),
        toProcessString("--allow-overwrite=true"),
        toProcessString("--auto-file-renaming=false"),
        toProcessString("--file-allocation=none"),
        toProcessString("--header=User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130.0.0.0 Safari/537.36"),
    };
}

class AriaTestEnvironment final : public testing::Environment
{
public:
    void SetUp() override
    {
        LoggerRegisterHelpper::registerLogger("Aria2Net", kRuntimeDir.string() + "/log/Aria2Net.log");
        LoggerRegisterHelpper::registerLogger("FFmpeg", kRuntimeDir.string() + "/log/FFmpeg.log");
        LoggerRegisterHelpper::registerLogger("Network", kRuntimeDir.string() + "/log/Network.log");
        LoggerRegisterHelpper::registerLogger("Download", kRuntimeDir.string() + "/log/Download.log");

        const auto exeDir = getModulePath();
        const auto ariaExecutable = std::filesystem::path(exeDir) / "aria" / ariaExecutableName();
        ASSERT_TRUE(std::filesystem::exists(ariaExecutable)) << "Cannot find aria2c in test output aria directory";

        auto args = ariaArguments(std::filesystem::path(exeDir) / kRuntimeDir);
        args.insert(args.begin(), PathToProcessString(ariaExecutable));
        TinyProcessLib::Config config;
        config.show_window = TinyProcessLib::Config::ShowWindow::hide;
        aria2Process_ = std::make_unique<TinyProcessLib::Process>(args, toProcessString(exeDir), nullptr, nullptr, false, config);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int exitStatus = 0;
        ASSERT_FALSE(aria2Process_->try_get_exit_status(exitStatus)) << "aria2c exited immediately";
    }

    void TearDown() override
    {
        if (!aria2Process_)
        {
            return;
        }

        aria2Process_->kill(true);
        aria2Process_.reset();
    }

private:
    std::unique_ptr<TinyProcessLib::Process> aria2Process_;
};

testing::Environment* const kAriaTestEnvironment = testing::AddGlobalTestEnvironment(new AriaTestEnvironment);

json loadCases()
{
    std::ifstream input(kCaseFile);
    if (!input)
    {
        throw std::runtime_error("Cannot open BiliBili flow case file: " + kCaseFile.string());
    }

    auto cases = json::parse(input);
    if (!cases.is_array() || cases.empty())
    {
        throw std::runtime_error("BiliBili flow case file must contain a non-empty JSON array");
    }
    return cases;
}

void expectView(const adapter::BaseVideoView& actual, const json& expected)
{
    if (expected.contains("identifier"))
    {
        EXPECT_EQ(actual.Identifier, expected.at("identifier").get<std::string>());
    }
    if (expected.contains("idType"))
    {
        EXPECT_EQ(actual.IdType, expected.at("idType").get<std::string>());
    }
    if (expected.contains("pluginId"))
    {
        EXPECT_EQ(actual.pluginId, expected.at("pluginId").get<int>());
    }
    if (expected.contains("fileType"))
    {
        EXPECT_EQ(actual.fileType, expected.at("fileType").get<adapter::FileType>());
    }
    if (expected.contains("fileExtension"))
    {
        EXPECT_EQ(actual.fileExtension, expected.at("fileExtension").get<std::string>());
    }

    EXPECT_FALSE(actual.Title.empty());
    EXPECT_FALSE(actual.Publisher.empty());
    EXPECT_FALSE(actual.Duration.empty());
    EXPECT_FALSE(actual.Option1.empty());
    EXPECT_FALSE(actual.Option2.empty());
}

void runFlowCase(plugin::IPlugin& plugin, const json& testCase)
{
    const auto url = testCase.at("url").get<std::string>();
    SCOPED_TRACE("url=" + url);

    ASSERT_TRUE(plugin.canParseUrl(url));
    const auto views = plugin.getVideoView(url);
    if (views.empty())
    {
        GTEST_SKIP() << "BiliBili view lookup is network dependent: " << url;
    }

    const auto& expectedViews = testCase.at("expectedViews");
    ASSERT_TRUE(expectedViews.is_array());
    ASSERT_GE(views.size(), expectedViews.size());
    for (std::size_t index = 0; index < expectedViews.size(); ++index)
    {
        expectView(views[index], expectedViews[index]);
    }

    VideoInfoFull videoInfo;
    videoInfo.downloadConfig = std::make_shared<DownloadConfig>(testCase.at("downloadConfig"));
    videoInfo.videoView = std::make_shared<adapter::BaseVideoView>(views.front());
    videoInfo.downloadConfig->downloadDir = std::filesystem::absolute(std::filesystem::path(videoInfo.downloadConfig->downloadDir)).string();

    auto downloader = plugin.getDownloader(videoInfo);
    const bool expectedCreated = testCase.at("expectedDownloader").at("created").get<bool>();
    EXPECT_EQ(downloader != nullptr, expectedCreated);
    if (!downloader)
    {
        GTEST_SKIP() << "BiliBili downloader creation is network dependent: " << url;
    }

    downloader->start();
    ASSERT_NE(downloader->status(), download::AbstractDownloader::Error);

    const auto timeout = std::chrono::seconds(testCase.value("timeoutSeconds", 600));
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        downloader->downloadStatus();
        const auto& info = downloader->info();
        std::cout << "Bili download: status=" << download::statusToString(downloader->status()) << " complete=" << info.complete << " total=" << info.total
                  << " speed=" << info.speed << " stage=" << info.stage << std::endl;

        if (downloader->status() == download::AbstractDownloader::Finished)
        {
            break;
        }
        ASSERT_NE(downloader->status(), download::AbstractDownloader::Error);
        std::this_thread::sleep_for(kDownloadPollInterval);
    }

    const auto& expectedDownload = testCase.at("expectedDownload");
    EXPECT_EQ(download::statusToString(downloader->status()), expectedDownload.at("status").get<std::string>());
    if (expectedDownload.value("fileExists", false))
    {
        EXPECT_TRUE(std::filesystem::exists(std::filesystem::path(downloader->path()) / util::utf8ToLocale(downloader->filename())));
    }
}
}  // namespace

TEST(BiliBiliPluginArtifactTest, ExportedInterfaces)
{
    initDir((kRuntimeDir.string() + "/").c_str());

    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);
    const PluginDeinitGuard deinit;
    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);

    const auto& info = plugin->pluginMessage();
    EXPECT_EQ(info.name, "BiliBili");
    EXPECT_EQ(info.pluginId, 1);
    EXPECT_EQ(info.domain, "https://www.bilibili.com");
    EXPECT_FALSE(plugin->websiteIcon().empty());
    for (const auto* url : kSupportedUrls)
    {
        EXPECT_TRUE(plugin->canParseUrl(url)) << url;
    }
}

TEST(BiliBiliPluginArtifactTest, BusinessFlowsMatchJsonCases)
{
    initDir((kRuntimeDir.string() + "/").c_str());

    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);
    const PluginDeinitGuard deinit;
    auto plugin = reinterpret_cast<plugin::IPlugin*>(handle);

    for (const auto& testCase : loadCases())
    {
        runFlowCase(*plugin, testCase);
    }
}
