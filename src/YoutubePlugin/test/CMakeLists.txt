include(${CMAKE_SOURCE_DIR}/test/testUtils.cmake)
EnableTest(YoutubePlugin)

set(TestCaseName ${PluginName}Test)

add_executable(${TestCaseName})

AddSourceFiles()
AddLibrary(
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        CURL::libcurl
)
