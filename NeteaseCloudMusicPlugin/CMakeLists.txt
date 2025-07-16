cmake_minimum_required(VERSION 3.15)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
set(VCPKG_APPLOCAL_DEPS ON)

project(NeteaseCloudMusicPlugin VERSION 1.0.0 LANGUAGES CXX)

# if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
# endif()

# set(CMAKE_CXX_FLAGS "-U__FILE__ -D__FILE__='\"$(subst $(dir $<),,$<)\"'")
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wno-builtin-macro-redefined SUPPORT_CXX_WNO_BUILTIN_MACRO_REDEFINED)
if (SUPPORT_CXX_WNO_BUILTIN_MACRO_REDEFINED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-builtin-macro-redefined")
else()
    message("CXXCompiler can't support Wno-builtin-macro-redefined")
endif (SUPPORT_CXX_WNO_BUILTIN_MACRO_REDEFINED)

# MSVC并行编译 cmake -j xxx 对于MSVC没有作用
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->) #高版本已被废弃，但是低版本的Gm会影响并行
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
# 寻找OpenSSL库
find_package(OpenSSL REQUIRED)

# 如果OpenSSL被找到，包含OpenSSL的头文件目录
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp" "rc/*.h")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE release CACHE STRING "Build Type" FORCE)
endif()

option(ENABLE_TEST "Enable Test" OFF)
if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

add_compile_definitions(PLUGIN_DLL_EXPORTS)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# file(READ "${CMAKE_CURRENT_SOURCE_DIR}/resource/deviceid.txt" DEVICE_DATA_TEXT)
# configure_file(
#         "${CMAKE_CURRENT_SOURCE_DIR}/src/NeteaseCloudMusicApi/DeviceIDData.h.in"
#         "${CMAKE_CURRENT_BINARY_DIR}/DeviceIDData.h"
#         @ONLY
# )
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/ThirdParty/spdlog
        ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon/PluginCommon
        ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
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



# install
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "$<TARGET_FILE:spdlog>" DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
install(FILES "$<TARGET_FILE:FFmpeg>" DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})

