set(TestTarget DedaoPlugin)

set(StaticTestTarget ${TestTarget}_static)
set(PluginArtifactTest ${TestTarget}_plugin_test)
set(PluginUnitTest ${TestTarget}_unit_test)

find_package(GTest CONFIG REQUIRED)
enable_testing()

set(TestIncludeDirectories
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/spdlog
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/sVanillaPluginCommon/PluginCommon
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/sVanillaPluginCommon
)

function(configure_plugin_test_target test_target)
    target_include_directories(${test_target} PRIVATE ${TestIncludeDirectories})
    target_compile_options(${test_target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
    target_link_libraries(${test_target} PRIVATE GTest::gtest_main)

    foreach(runtime_target spdlog FFmpeg)
        if (TARGET ${runtime_target})
            add_custom_command(TARGET ${test_target}
                POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${runtime_target}> $<TARGET_FILE_DIR:${test_target}>
            )
        endif()
    endforeach()
endfunction()

file(GLOB_RECURSE PluginTestSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/*.cpp"
)

if (PluginTestSources)
    add_executable(${PluginArtifactTest} ${PluginTestSources})
    set_target_properties(${PluginArtifactTest} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:${TestTarget}>
    )
    add_test(NAME ${PluginArtifactTest} COMMAND ${PluginArtifactTest})
    set_tests_properties(${PluginArtifactTest} PROPERTIES
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${PluginArtifactTest}>
    )

    set(DYNAMIC_LIBS
        CURL::libcurl
        spdlog
        ${OPENSSL_LIBRARIES}
        FFmpeg
    )

    set(STATIC_LIBS
        AriaClient
        NetWork
        Util
        PluginDownload
        PluginCommon
        nlohmann_json::nlohmann_json
    )

    target_link_libraries(${PluginArtifactTest} PRIVATE ${DYNAMIC_LIBS})
    if(UNIX AND NOT APPLE)
        target_link_libraries(${PluginArtifactTest}
            PRIVATE
                -Wl,--whole-archive ${STATIC_LIBS} -Wl,--no-whole-archive
        )
    else()
        target_link_libraries(${PluginArtifactTest}
            PRIVATE
                ${STATIC_LIBS}
        )
    endif()

    configure_plugin_test_target(${PluginArtifactTest})
    target_link_libraries(${PluginArtifactTest} PRIVATE ${TestTarget})

    add_custom_command(TARGET ${PluginArtifactTest}
        POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${TestTarget}> $<TARGET_FILE_DIR:${PluginArtifactTest}>
    )
endif()


file(GLOB_RECURSE UnitTestSource CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp"
)
if (UnitTestSource)
    if (NOT TARGET ${StaticTestTarget})
        message(FATAL_ERROR "${StaticTestTarget} must be defined by ${TestTarget}/CMakeLists.txt before adding tests")
    endif()

    add_executable(${PluginUnitTest} ${UnitTestSource})
    add_test(NAME ${PluginUnitTest} COMMAND ${PluginUnitTest})
    configure_plugin_test_target(${PluginUnitTest})
    target_compile_definitions(${PluginUnitTest} PRIVATE PLUGIN_LOCAL)
    target_link_libraries(${PluginUnitTest} PRIVATE ${StaticTestTarget})
endif()

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

        if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
            set(VCPKG_LIB ${VCPAKG_DEP_DIR}/debug/${LIB_PRX_PATH}/)
        else ()
            set(VCPKG_LIB ${VCPAKG_DEP_DIR}/${LIB_PRX_PATH}/)
        endif ()

        string(FIND "${VCPAKG_DEP_DIR}" "gtest" POS)
        if (EXISTS "${VCPKG_LIB}" AND "${POS}" EQUAL -1)
            foreach(TEST_EXECUTABLE ${PluginArtifactTest} ${PluginUnitTest})
                if (TARGET ${TEST_EXECUTABLE})
                    add_custom_command(TARGET ${TEST_EXECUTABLE}
                        POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy_directory ${VCPKG_LIB} $<TARGET_FILE_DIR:${TEST_EXECUTABLE}>
                    )
                endif()
            endforeach()
        endif ()
    endforeach ()
endif ()