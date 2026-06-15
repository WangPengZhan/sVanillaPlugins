set(TestTarget XHSPlugin)
set(StaticTestTarget ${TestTarget}_static)
set(PluginArtifactTest ${TestTarget}_plugin_test)
set(PluginUnitTest ${TestTarget}_unit_test)

find_package(GTest CONFIG REQUIRED)

enable_testing()

set(TEST_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_SOURCE_DIR}/ThirdParty/spdlog
    ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon/PluginCommon
    ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

function(configure_test_target test_target)
    target_include_directories(${test_target}
        PRIVATE
            ${TEST_INCLUDE_DIRECTORIES}
    )

    target_link_libraries(${test_target}
        PRIVATE
            GTest::gtest_main
    )

    if (TARGET spdlog)
        add_custom_command(TARGET ${test_target}
            POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:spdlog> $<TARGET_FILE_DIR:${test_target}>
        )
    endif()

    if (TARGET FFmpeg)
        add_custom_command(TARGET ${test_target}
            POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:FFmpeg> $<TARGET_FILE_DIR:${test_target}>
        )
    endif()
endfunction()

file(GLOB_RECURSE PLUGIN_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/*.cpp"
)

if (PLUGIN_TEST_SOURCES)
    add_executable(${PluginArtifactTest} ${PLUGIN_TEST_SOURCES})
    set_target_properties(${PluginArtifactTest} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:${TestTarget}>
    )
    add_test(NAME ${PluginArtifactTest} COMMAND ${PluginArtifactTest})

    configure_test_target(${PluginArtifactTest})

    target_link_libraries(${PluginArtifactTest}
        PRIVATE
            ${TestTarget}
    )

    add_custom_command(TARGET ${PluginArtifactTest}
        POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${TestTarget}> $<TARGET_FILE_DIR:${PluginArtifactTest}>
    )
endif()

file(GLOB_RECURSE UNIT_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp"
)

if (UNIT_TEST_SOURCES)
    add_executable(${PluginUnitTest} ${UNIT_TEST_SOURCES})
    add_test(NAME ${PluginUnitTest} COMMAND ${PluginUnitTest})

    configure_test_target(${PluginUnitTest})

    target_compile_definitions(${PluginUnitTest}
        PRIVATE
            PLUGIN_LOCAL
    )

    target_link_libraries(${PluginUnitTest}
        PRIVATE
            ${StaticTestTarget}
    )
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
            install(DIRECTORY ${VCPKG_LIB} DESTINATION ${CMAKE_INSTALL_BINDIR})
        endif ()
    endforeach ()
endif ()
