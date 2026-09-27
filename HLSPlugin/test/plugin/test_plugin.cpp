#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "Aria2Net/AriaClient/AriaClient.h"
#include "IPlugin.h"
#include "LoggerRegisterHelpper.h"
#include "TemplatePluginCall.h"
#include "Util/LocaleHelper.h"
#include "Util/TimerUtil.h"
#include "Util/process.hpp"

namespace
{
constexpr auto kPollInterval = std::chrono::seconds(1);
const std::filesystem::path kRuntimeDir = "hls-plugin-test/runtime";
const std::filesystem::path kCaseFile = "business_flow_cases.json";

std::filesystem::path ariaExecutableName()
{
#ifdef _WIN32
    return "aria2c.exe";
#else
    return "aria2c";
#endif
}

TinyProcessLib::Process::string_type toProcessString(const std::filesystem::path& value)
{
#if defined(_WIN32) && defined(UNICODE)
    return value.wstring();
#else
    return value.string();
#endif
}

bool ariaIsRunning()
{
    return !aria2net::AriaClient::globalClient().GetAriaVersionAsync().result.version.empty();
}

class AriaTestEnvironment final : public testing::Environment
{
public:
    void SetUp() override
    {
        std::filesystem::create_directories(kRuntimeDir / "log");
        LoggerRegisterHelpper::registerLogger("Aria2Net", (kRuntimeDir / "log/Aria2Net.log").string());
        LoggerRegisterHelpper::registerLogger("FFmpeg", (kRuntimeDir / "log/FFmpeg.log").string());
        LoggerRegisterHelpper::registerLogger("Network", (kRuntimeDir / "log/Network.log").string());
        LoggerRegisterHelpper::registerLogger("Download", (kRuntimeDir / "log/Download.log").string());
        if (ariaIsRunning())
        {
            return;
        }

        const auto executableDir = std::filesystem::path(getModulePath());
        const auto ariaExecutable = executableDir / "aria" / ariaExecutableName();
        ASSERT_TRUE(std::filesystem::exists(ariaExecutable));

        const auto ariaDir = executableDir / kRuntimeDir / "aria";
        std::filesystem::create_directories(ariaDir);
        const auto sessionFile = ariaDir / "aria.session";
        std::ofstream(sessionFile, std::ios::app).close();
        std::vector<TinyProcessLib::Process::string_type> args = {
            toProcessString(ariaExecutable),
            toProcessString("--enable-rpc"),
            toProcessString("--rpc-listen-all=false"),
            toProcessString("--rpc-listen-port=6800"),
            toProcessString("--rpc-secret=sVanilla"),
            toProcessString("--input-file=" + sessionFile.string()),
            toProcessString("--save-session=" + sessionFile.string()),
            toProcessString("--log=" + (ariaDir / "aria.log").string()),
            toProcessString("--max-concurrent-downloads=6"),
            toProcessString("--max-connection-per-server=16"),
            toProcessString("--split=5"),
            toProcessString("--continue=true"),
            toProcessString("--allow-overwrite=true"),
            toProcessString("--auto-file-renaming=false"),
            toProcessString("--file-allocation=none"),
        };
        TinyProcessLib::Config config;
        config.show_window = TinyProcessLib::Config::ShowWindow::hide;
        process_ = std::make_unique<TinyProcessLib::Process>(args, toProcessString(executableDir), nullptr, nullptr, false, config);

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (std::chrono::steady_clock::now() < deadline && !ariaIsRunning())
        {
            int exitStatus = 0;
            ASSERT_FALSE(process_->try_get_exit_status(exitStatus));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        ASSERT_TRUE(ariaIsRunning());
    }

    void TearDown() override
    {
        if (process_)
        {
            process_->kill(true);
        }
    }

private:
    std::unique_ptr<TinyProcessLib::Process> process_;
};

testing::Environment* const kAriaEnvironment = testing::AddGlobalTestEnvironment(new AriaTestEnvironment);

struct PluginGuard
{
    ~PluginGuard()
    {
        pluginDeinit();
        deinit();
    }
};

struct ExpectedDownloader
{
    bool created{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ExpectedDownloader, created)
};

struct ExpectedDownload
{
    std::string status;
    bool fileExists{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ExpectedDownload, status, fileExists)
};

struct BusinessFlowCase
{
    std::string url;
    std::string description;
    bool isSmokeTest{};
    std::string linkType;
    adapter::VideoView expectedViews;
    DownloadConfig downloadConfig;
    ExpectedDownloader expectedDownloader;
    ExpectedDownload expectedDownload;
    int timeoutSeconds{600};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BusinessFlowCase, url, description, isSmokeTest, linkType, expectedViews, downloadConfig, expectedDownloader,
                                                expectedDownload, timeoutSeconds)
};

const std::vector<BusinessFlowCase>& loadCases()
{
    static const auto cases = [] {
        std::ifstream input(kCaseFile);
        if (!input)
        {
            throw std::runtime_error("Cannot open business-flow case file: " + kCaseFile.string());
        }
        const auto json = nlohmann::json::parse(input);
        if (!json.is_array() || json.empty())
        {
            throw std::runtime_error("Business-flow case file must contain a non-empty array");
        }
        const auto parsed = json.get<std::vector<BusinessFlowCase>>();
        std::map<std::string, std::size_t> countPerType;
        std::size_t smokeCount = 0;
        for (const auto& testCase : parsed)
        {
            if (testCase.url.empty() || testCase.description.empty() || testCase.linkType.empty() || testCase.expectedViews.empty())
            {
                throw std::runtime_error("Every business-flow case requires metadata and expectedViews");
            }
            ++countPerType[testCase.linkType];
            smokeCount += testCase.isSmokeTest ? 1 : 0;
        }
        if (smokeCount == 0)
        {
            throw std::runtime_error("At least one smoke case is required");
        }
        return parsed;
    }();
    return cases;
}

void expectView(const adapter::BaseVideoView& actual, const adapter::BaseVideoView& expected)
{
    if (!expected.Identifier.empty() && expected.Identifier != "placeholder")
    {
        EXPECT_EQ(actual.Identifier, expected.Identifier);
    }
    if (!expected.IdType.empty())
    {
        EXPECT_EQ(actual.IdType, expected.IdType);
    }
    EXPECT_EQ(actual.pluginId, expected.pluginId);
    if (expected.fileType != adapter::FileType::Unknow)
    {
        EXPECT_EQ(actual.fileType, expected.fileType);
    }
    if (!expected.fileExtension.empty())
    {
        EXPECT_EQ(actual.fileExtension, expected.fileExtension);
    }
}

void downloadView(plugin::IPlugin& plugin, const BusinessFlowCase& testCase, const adapter::BaseVideoView& view)
{
    VideoInfoFull videoInfo;
    videoInfo.downloadConfig = std::make_shared<DownloadConfig>(testCase.downloadConfig);
    videoInfo.videoView = std::make_shared<adapter::BaseVideoView>(view);
    videoInfo.downloadConfig->downloadDir = std::filesystem::absolute(videoInfo.downloadConfig->downloadDir).string();

    auto downloader = plugin.getDownloader(videoInfo);
    ASSERT_EQ(downloader != nullptr, testCase.expectedDownloader.created);
    ASSERT_NE(downloader, nullptr);

    downloader->start();
    ASSERT_NE(downloader->status(), download::AbstractDownloader::Error);

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(testCase.timeoutSeconds);
    while (std::chrono::steady_clock::now() < deadline)
    {
        downloader->downloadStatus();
        const auto& info = downloader->info();
        std::cout << testCase.linkType << ": status=" << download::statusToString(downloader->status()) << " complete=" << info.complete
                  << " total=" << info.total << " speed=" << info.speed << " stage=" << info.stage << '\n';
        if (downloader->status() == download::AbstractDownloader::Finished)
        {
            break;
        }
        ASSERT_NE(downloader->status(), download::AbstractDownloader::Error);
        std::this_thread::sleep_for(kPollInterval);
    }

    EXPECT_EQ(download::statusToString(downloader->status()), testCase.expectedDownload.status);
    if (testCase.expectedDownload.fileExists)
    {
        EXPECT_TRUE(std::filesystem::exists(std::filesystem::path(downloader->path()) / util::utf8ToLocale(downloader->filename())));
    }
}

void runCase(plugin::IPlugin& plugin, const BusinessFlowCase& testCase)
{
    SCOPED_TRACE(testCase.description + ": " + testCase.url);
    ASSERT_TRUE(plugin.canParseUrl(testCase.url));

    const auto views = plugin.getVideoView(testCase.url);
    ASSERT_FALSE(views.empty());
    ASSERT_GE(views.size(), testCase.expectedViews.size());
    for (std::size_t index = 0; index < testCase.expectedViews.size(); ++index)
    {
        expectView(views[index], testCase.expectedViews[index]);
    }
    for (const auto& view : views)
    {
        downloadView(plugin, testCase, view);
    }
}

template <typename Predicate>
void runCases(Predicate predicate)
{
    initDir((kRuntimeDir.string() + "/").c_str());
    const auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);
    const PluginGuard guard;
    auto* plugin = reinterpret_cast<plugin::IPlugin*>(handle);

    bool found = false;
    for (const auto& testCase : loadCases())
    {
        if (predicate(testCase))
        {
            found = true;
            runCase(*plugin, testCase);
        }
    }
    ASSERT_TRUE(found);
}

void runLinkTypeCases(const std::string& linkType)
{
    runCases([&linkType](const BusinessFlowCase& testCase) {
        return testCase.linkType == linkType;
    });
}
}  // namespace

TEST(HLSPluginBusinessTesting, SingleTest)
{
    initDir((kRuntimeDir.string() + "/").c_str());
    const auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);
    const PluginGuard guard;
    auto* plugin = reinterpret_cast<plugin::IPlugin*>(handle);

    BusinessFlowCase testCase;
    testCase.url = "https://devstreaming-cdn.apple.com/videos/streaming/examples/img_bipbop_adv_example_ts/master.m3u8";
    testCase.description = "Single URL download";
    testCase.linkType = "MediaPlaylist";
    testCase.downloadConfig.downloadDir = "hls-plugin-test/download/";
    testCase.expectedDownloader.created = true;
    testCase.expectedDownload.status = "Finished";
    testCase.expectedDownload.fileExists = true;

    runCase(*plugin, testCase);
}

TEST(HLSPluginArtifactTest, ExportedInterfaces)
{
    initDir((kRuntimeDir.string() + "/").c_str());
    const auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);
    const PluginGuard guard;
    auto* plugin = reinterpret_cast<plugin::IPlugin*>(handle);

    const auto& info = plugin->pluginMessage();
    EXPECT_EQ(info.name, "HLSStream");
    EXPECT_EQ(info.pluginId, 3);
    EXPECT_EQ(info.domain, "https://developer.apple.com/streaming/");
    EXPECT_FALSE(plugin->websiteIcon().empty());
}

TEST(HLSPluginBusinessFlowTest, MediaPlaylist)
{
    runLinkTypeCases("MediaPlaylist");
}

TEST(HLSPluginBusinessFlowTest, MasterPlaylist)
{
    runLinkTypeCases("MasterPlaylist");
}

TEST(HLSPluginBusinessFlowTest, Smoke)
{
    runCases([](const BusinessFlowCase& testCase) {
        return testCase.isSmokeTest;
    });
}
