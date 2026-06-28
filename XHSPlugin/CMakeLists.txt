cmake_minimum_required(VERSION 3.15)

project(XHSPlugin VERSION 1.0.0 LANGUAGES CXX)
include(GNUInstallDirs)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wno-builtin-macro-redefined SUPPORT_CXX_WNO_BUILTIN_MACRO_REDEFINED)
if (SUPPORT_CXX_WNO_BUILTIN_MACRO_REDEFINED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-builtin-macro-redefined")
else()
    message("CXXCompiler can't support Wno-builtin-macro-redefined")
endif (SUPPORT_CXX_WNO_BUILTIN_MACRO_REDEFINED)

# MSVC parallel compile options.
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->)
cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)

add_compile_definitions(UNICODE)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

cmake_policy(SET CMP0087 NEW)

find_package(nlohmann_json CONFIG REQUIRED)
set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE release CACHE STRING "Build Type" FORCE)
endif()

option(ENABLE_TEST "Enable Test" OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(PROJECT_NAME_STATIC ${PROJECT_NAME}_static)

set(INCLUDE_DIRECTORIES_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spdlog
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/sVanillaPluginCommon/PluginCommon
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/sVanillaPluginCommon
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
    PluginCrypto
    nlohmann_json::nlohmann_json
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        PLUGIN_DLL_EXPORTS
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${INCLUDE_DIRECTORIES_PRIVATE}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${DYNAMIC_LIBS}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            -Wl,--whole-archive ${STATIC_LIBS} -Wl,--no-whole-archive
    )
else()
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            ${STATIC_LIBS}
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:spdlog> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:FFmpeg> $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

if (ENABLE_TEST)
    add_library(${PROJECT_NAME_STATIC} STATIC ${SOURCES})
    target_compile_definitions(${PROJECT_NAME_STATIC}
        PRIVATE
            PLUGIN_LOCAL
    )

    target_include_directories(${PROJECT_NAME_STATIC}
        PRIVATE
            ${INCLUDE_DIRECTORIES_PRIVATE}
    )

    target_link_libraries(${PROJECT_NAME_STATIC}
        PRIVATE
            ${DYNAMIC_LIBS}
    )

    if(UNIX AND NOT APPLE)
        target_link_libraries(${PROJECT_NAME_STATIC}
            PRIVATE
                -Wl,--whole-archive ${STATIC_LIBS} -Wl,--no-whole-archive
        )
    else()
        target_link_libraries(${PROJECT_NAME_STATIC}
            PRIVATE
                ${STATIC_LIBS}
        )
    endif()

    enable_testing()
    add_subdirectory(test)
endif()

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "$<TARGET_FILE:spdlog>" DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "$<TARGET_FILE:FFmpeg>" DESTINATION ${CMAKE_INSTALL_BINDIR})
