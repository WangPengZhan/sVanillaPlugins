set(TestTarget YoutubePlugin)
set(PluginTest ${TestTarget}_test)
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)
set(SOURCES ${TEST_SOURCES} ${TESTED_SOURCES} ${TESTED_DEPANDED_SOURCES})

find_package(GTest CONFIG REQUIRED)

enable_testing()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_executable(${PluginTest} ${SOURCES})
set_target_properties(${PluginTest} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:${TestTarget}>
)
add_test(NAME ${PluginTest} COMMAND ${PluginTest})

target_include_directories(${PluginTest}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_SOURCE_DIR}/ThirdParty/spdlog
        ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon/PluginCommon
        ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon
)

target_link_libraries(${PluginTest}
    PRIVATE
        ${TestTarget}
        GTest::gtest_main 
)

add_custom_command(TARGET ${PluginTest}
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:spdlog> $<TARGET_FILE_DIR:${PluginTest}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:FFmpeg> $<TARGET_FILE_DIR:${PluginTest}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${TestTarget}> $<TARGET_FILE_DIR:${PluginTest}>
)

file(GLOB VCPAKG_DEP_DIRS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/ThirdParty/vcpkg/packages/*")
if (VCPAKG_DEP_DIRS)
    foreach (VCPAKG_DEP_DIR ${VCPAKG_DEP_DIRS})
        if (WIN32)
            set(LIB_PRX_PATH bin)
        elseif (APPLE)
            set(LIB_PRX_PATH lib)
        elseif (UNIX AND NOT APPLE)
            set(LIB_PRX_PATH lib)
        endif ()

        if (${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "DEBUG")
            set(VCPKG_LIB ${VCPAKG_DEP_DIR}/debug/${LIB_PRX_PATH}/)
        else ()
            set(VCPKG_LIB ${VCPAKG_DEP_DIR}/${LIB_PRX_PATH}/)
        endif ()

        string(FIND "${VCPAKG_DEP_DIR}" "gtest" POS)
        if (EXISTS "${VCPKG_LIB}" AND "${POS}" EQUAL -1)
            add_custom_command(TARGET ${PluginTest}
                POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${VCPKG_LIB} $<TARGET_FILE_DIR:${PluginTest}>
            )
            install(DIRECTORY ${VCPKG_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
        endif ()
    endforeach ()
endif ()
