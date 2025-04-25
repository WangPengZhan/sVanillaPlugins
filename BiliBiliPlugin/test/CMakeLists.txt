set(PluginTest BiliBiliPlugin_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)
set(SOURCES ${TEST_SOURCES} ${TESTED_SOURCES} ${TESTED_DEPANDED_SOURCES})

find_package(GTest CONFIG REQUIRED)

enable_testing()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_executable(${PluginTest} ${SOURCES})
add_test(NAME ${PluginTest} COMMAND ${PluginTest})

target_include_directories(${PluginTest}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/../src
        ${CMAKE_SOURCE_DIR}/ThirdParty/spdlog
        ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon/PluginCommon
        ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon
)

target_link_libraries(${PluginTest}
    PRIVATE
        TemplatePlugin
        GTest::gtest_main 
)

add_custom_command(TARGET ${PluginTest}
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:spdlog> $<TARGET_FILE_DIR:${PluginTest}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:FFmpeg> $<TARGET_FILE_DIR:${PluginTest}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:TemplatePlugin> $<TARGET_FILE_DIR:${PluginTest}>
)