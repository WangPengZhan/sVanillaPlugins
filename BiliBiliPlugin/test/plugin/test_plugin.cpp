#include <array>
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

#include "IPlugin.h"
#include "TemplatePluginCall.h"
#include "Aria2Net/AriaClient/AriaClient.h"
#include "LoggerRegisterHelpper.h"
#include "Util/TimerUtil.h"
#include "Util/process.hpp"
#include "Util/LocaleHelper.h"

namespace
{

constexpr auto kDownloadPollInterval = std::chrono::seconds(1);
const std::filesystem::path kRuntimeDir = "bili-plugin-test/runtime";
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
        LoggerRegisterHelpper::registerLogger("Aria2Net", kRuntimeDir.string() + "/log/Aria2Net.log");
        LoggerRegisterHelpper::registerLogger("FFmpeg", kRuntimeDir.string() + "/log/FFmpeg.log");
        LoggerRegisterHelpper::registerLogger("Network", kRuntimeDir.string() + "/log/Network.log");
        LoggerRegisterHelpper::registerLogger("Download", kRuntimeDir.string() + "/log/Download.log");
        if (ariaIsRunning())
        {
            return;
        }

        const auto exeDir = std::filesystem::path(getModulePath());
        const auto ariaExecutable = exeDir / "aria" / ariaExecutableName();
        ASSERT_TRUE(std::filesystem::exists(ariaExecutable)) << "Cannot find aria2c in test output aria directory";

        const auto ariaDir = exeDir / kRuntimeDir / "aria";
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
        aria2Process_ = std::make_unique<TinyProcessLib::Process>(args, toProcessString(exeDir), nullptr, nullptr, false, config);

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (std::chrono::steady_clock::now() < deadline && !ariaIsRunning())
        {
            int exitStatus = 0;
            ASSERT_FALSE(aria2Process_->try_get_exit_status(exitStatus)) << "aria2c exited during startup";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        ASSERT_TRUE(ariaIsRunning()) << "aria2c RPC did not become ready within five seconds";
    }

    void TearDown() override
    {
        if (aria2Process_)
        {
            aria2Process_->kill(true);
        }
    }

private:
    std::unique_ptr<TinyProcessLib::Process> aria2Process_;
};

testing::Environment* const kAriaTestEnvironment = testing::AddGlobalTestEnvironment(new AriaTestEnvironment);

struct PluginDeinitGuard
{
    ~PluginDeinitGuard()
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
    bool isSmokeTest{};
    std::string linkType;
    std::string description;
    std::string url;
    DownloadConfig downloadConfig;
    adapter::VideoView expectedViews;
    ExpectedDownloader expectedDownloader;
    ExpectedDownload expectedDownload;
    int timeoutSeconds{600};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BusinessFlowCase, isSmokeTest, linkType, description, url, downloadConfig, expectedViews, expectedDownloader,
                                                expectedDownload, timeoutSeconds)
};

const std::vector<BusinessFlowCase>& loadCases()
{
    static const auto cases = [] {
        std::ifstream input(kCaseFile);
        if (!input)
        {
            throw std::runtime_error("Cannot open BiliBili flow case file: " + kCaseFile.string());
        }

        const auto caseJson = nlohmann::json::parse(input);
        if (!caseJson.is_array() || caseJson.empty())
        {
            throw std::runtime_error("BiliBili flow case file must contain a non-empty JSON array");
        }
        const auto parsedCases = caseJson.get<std::vector<BusinessFlowCase>>();
        std::map<std::string, std::size_t> casesPerLinkType;
        std::size_t smokeCaseCount = 0;
        for (const auto& testCase : parsedCases)
        {
            if (testCase.linkType.empty() || testCase.description.empty() || testCase.url.empty())
            {
                throw std::runtime_error("Every BiliBili flow case requires isSmokeTest, linkType, and description metadata");
            }
            ++casesPerLinkType[testCase.linkType];
            if (testCase.expectedViews.empty())
            {
                throw std::runtime_error("Every BiliBili flow case requires at least one expected view");
            }
            for (const auto& expectedView : testCase.expectedViews)
            {
                if (expectedView.IdType.empty() || expectedView.pluginId < 0 || expectedView.fileType == adapter::FileType::Unknow ||
                    expectedView.fileExtension.empty())
                {
                    throw std::runtime_error("Every expected BiliBili view requires idType, pluginId, fileType, and fileExtension");
                }
            }
            if (testCase.isSmokeTest)
            {
                ++smokeCaseCount;
                if (testCase.downloadConfig.downloadDir.empty() || testCase.expectedDownload.status.empty())
                {
                    throw std::runtime_error("Every smoke case requires download expectations");
                }
            }
        }
        if (smokeCaseCount == 0)
        {
            throw std::runtime_error("BiliBili flow cases require at least one smoke case");
        }
        constexpr std::array expectedLinkTypes = {"Aid", "Bid", "BangumiSS", "BangumiEP", "BangumiMD", "CheeseSS", "CheeseEP", "FavoritesId", "UserId"};
        for (const auto* linkType : expectedLinkTypes)
        {
            if (casesPerLinkType[linkType] < 2)
            {
                throw std::runtime_error("BiliBili link type requires at least two cases: " + std::string(linkType));
            }
        }
        return parsedCases;
    }();
    return cases;
}

void expectView(const adapter::BaseVideoView& actual, const adapter::BaseVideoView& expected)
{
    if (!expected.Identifier.empty())
    {
        EXPECT_EQ(actual.Identifier, expected.Identifier);
    }
    EXPECT_EQ(actual.IdType, expected.IdType);
    EXPECT_EQ(actual.pluginId, expected.pluginId);
    EXPECT_EQ(actual.fileType, expected.fileType);
    EXPECT_EQ(actual.fileExtension, expected.fileExtension);
}

void runFlowCase(plugin::IPlugin& plugin, const BusinessFlowCase& testCase)
{
    SCOPED_TRACE("description=" + testCase.description + ", linkType=" + testCase.linkType + ", url=" + testCase.url);

    ASSERT_TRUE(plugin.canParseUrl(testCase.url));
    const auto views = plugin.getVideoView(testCase.url);
    if (views.empty())
    {
        GTEST_SKIP() << "BiliBili view lookup is network dependent: " << testCase.url;
    }

    ASSERT_GE(views.size(), testCase.expectedViews.size());
    for (std::size_t index = 0; index < testCase.expectedViews.size(); ++index)
    {
        expectView(views[index], testCase.expectedViews[index]);
    }

    VideoInfoFull videoInfo;
    videoInfo.downloadConfig = std::make_shared<DownloadConfig>(testCase.downloadConfig);
    videoInfo.videoView = std::make_shared<adapter::BaseVideoView>(views.front());
    videoInfo.downloadConfig->downloadDir = std::filesystem::absolute(std::filesystem::path(videoInfo.downloadConfig->downloadDir)).string();

    auto downloader = plugin.getDownloader(videoInfo);
    EXPECT_EQ(downloader != nullptr, testCase.expectedDownloader.created);
    if (!downloader)
    {
        GTEST_SKIP() << "BiliBili downloader creation is network dependent: " << testCase.url;
    }

    downloader->start();
    ASSERT_NE(downloader->status(), download::AbstractDownloader::Error);

    const auto timeout = std::chrono::seconds(testCase.timeoutSeconds);
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

    EXPECT_EQ(download::statusToString(downloader->status()), testCase.expectedDownload.status);
    if (testCase.expectedDownload.fileExists)
    {
        EXPECT_TRUE(std::filesystem::exists(std::filesystem::path(downloader->path()) / util::utf8ToLocale(downloader->filename())));
    }
}

void runLinkTypeCases(const std::string& linkType)
{
    initDir((kRuntimeDir.string() + "/").c_str());
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);
    const PluginDeinitGuard deinit;
    auto* plugin = reinterpret_cast<plugin::IPlugin*>(handle);

    bool found = false;
    for (const auto& testCase : loadCases())
    {
        if (testCase.linkType == linkType)
        {
            found = true;
            runFlowCase(*plugin, testCase);
        }
    }
    ASSERT_TRUE(found) << "No BiliBili flow cases for link type: " << linkType;
}

void runSmokeCases()
{
    initDir((kRuntimeDir.string() + "/").c_str());
    auto handle = pluginInit();
    ASSERT_NE(handle, nullptr);
    const PluginDeinitGuard deinit;
    auto* plugin = reinterpret_cast<plugin::IPlugin*>(handle);

    bool found = false;
    for (const auto& testCase : loadCases())
    {
        if (testCase.isSmokeTest)
        {
            found = true;
            runFlowCase(*plugin, testCase);
        }
    }
    ASSERT_TRUE(found) << "No BiliBili smoke flow cases";
}
}  // namespace

TEST(BiliBiliPluginBusinessFlowTest, Aid)
{
    runLinkTypeCases("Aid");
}

TEST(BiliBiliPluginBusinessFlowTest, Bid)
{
    runLinkTypeCases("Bid");
}

TEST(BiliBiliPluginBusinessFlowTest, BangumiSS)
{
    runLinkTypeCases("BangumiSS");
}

TEST(BiliBiliPluginBusinessFlowTest, BangumiEP)
{
    runLinkTypeCases("BangumiEP");
}

TEST(BiliBiliPluginBusinessFlowTest, BangumiMD)
{
    runLinkTypeCases("BangumiMD");
}

TEST(BiliBiliPluginBusinessFlowTest, CheeseSS)
{
    runLinkTypeCases("CheeseSS");
}

TEST(BiliBiliPluginBusinessFlowTest, CheeseEP)
{
    runLinkTypeCases("CheeseEP");
}

TEST(BiliBiliPluginBusinessFlowTest, FavoritesId)
{
    runLinkTypeCases("FavoritesId");
}

TEST(BiliBiliPluginBusinessFlowTest, UserId)
{
    runLinkTypeCases("UserId");
}

TEST(BiliBiliPluginBusinessFlowTest, Smoke)
{
    runSmokeCases();
}
