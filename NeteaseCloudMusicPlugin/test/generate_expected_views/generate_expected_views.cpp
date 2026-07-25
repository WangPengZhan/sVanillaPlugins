#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <nlohmann/json.hpp>

#include "IPlugin.h"
#include "LoggerRegisterHelpper.h"
#include "TemplatePluginCall.h"

namespace
{
using OrderedJson = nlohmann::ordered_json;

struct PluginGuard
{
    ~PluginGuard()
    {
        pluginDeinit();
        deinit();
    }
};

OrderedJson loadCases(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input)
    {
        throw std::runtime_error("Cannot open business-flow cases: " + path.string());
    }
    auto cases = OrderedJson::parse(input);
    if (!cases.is_array() || cases.empty())
    {
        throw std::runtime_error("Business-flow cases must contain a non-empty JSON array");
    }
    return cases;
}

void saveCases(const std::filesystem::path& path, const OrderedJson& cases)
{
    const auto temporaryPath = path.string() + ".tmp";
    {
        std::ofstream output(temporaryPath, std::ios::binary | std::ios::trunc);
        if (!output)
        {
            throw std::runtime_error("Cannot create temporary cases file: " + temporaryPath);
        }
        output << cases.dump(4, ' ', false, OrderedJson::error_handler_t::replace) << '\n';
    }
    std::filesystem::copy_file(temporaryPath, path, std::filesystem::copy_options::overwrite_existing);
    std::filesystem::remove(temporaryPath);
}

OrderedJson orderViews(const adapter::VideoView& views, const OrderedJson& fieldOrder)
{
    const auto generatedViews = OrderedJson::parse(nlohmann::json(views).dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
    if (!fieldOrder.is_object())
    {
        return generatedViews;
    }

    auto orderedViews = OrderedJson::array();
    for (const auto& generatedView : generatedViews)
    {
        auto orderedView = OrderedJson::object();
        for (const auto& item : fieldOrder.items())
        {
            if (generatedView.contains(item.key()))
            {
                orderedView[item.key()] = generatedView.at(item.key());
            }
        }
        for (const auto& [key, value] : generatedView.items())
        {
            if (!orderedView.contains(key))
            {
                orderedView[key] = value;
            }
        }
        orderedViews.push_back(std::move(orderedView));
    }
    return orderedViews;
}
}  // namespace

int main(int argc, char** argv)
{
    try
    {
        const auto casesPath = std::filesystem::absolute(argc < 2 ? "./business_flow_cases.json" : argv[1]);
        const std::string runtime = argc < 3 ? "./" : argv[2];

        LoggerRegisterHelpper::registerLogger("Aria2Net", runtime + "log/Aria2Net.log");
        LoggerRegisterHelpper::registerLogger("FFmpeg", runtime + "log/FFmpeg.log");
        LoggerRegisterHelpper::registerLogger("Network", runtime + "log/Network.log");
        LoggerRegisterHelpper::registerLogger("Download", runtime + "log/Download.log");

        initDir(runtime.c_str());
        const auto handle = pluginInit();
        if (!handle)
        {
            throw std::runtime_error("pluginInit returned null");
        }
        const PluginGuard guard;
        auto* plugin = reinterpret_cast<plugin::IPlugin*>(handle);

        auto cases = loadCases(casesPath);
        const auto fieldOrder = cases.front().at("expectedViews").front();
        std::size_t updated = 0;
        for (auto& testCase : cases)
        {
            const auto url = testCase.at("url").get<std::string>();
            try
            {
                const auto views = plugin->getVideoView(url);
                if (views.empty())
                {
                    std::cout << "Kept existing expectedViews because no views were returned: " << url << '\n';
                    continue;
                }
                testCase["expectedViews"] = orderViews(views, fieldOrder);
                ++updated;
                std::cout << "Updated " << url << " with " << views.size() << " views\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            catch (const std::exception& error)
            {
                std::cerr << "Kept existing expectedViews because view generation failed: " << url << ": " << error.what() << '\n';
            }
        }

        saveCases(casesPath, cases);
        std::cout << "Updated " << updated << '/' << cases.size() << " cases for " << plugin->pluginMessage().name << '\n';
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
