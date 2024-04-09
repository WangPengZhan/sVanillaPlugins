cmake_minimum_required(VERSION 3.15)

option(SINGLE_PLUGIN_BUILD "Single Plugin" OFF)
# set(SINGLE_PLUGIN_NAME "" CACHE STRING "Single Plugin Name")
option(ENABLE_ALL_TEST "Enable Test" OFF)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
set(VCPKG_APPLOCAL_DEPS ON)

project(sVanillaPlugins VERSION 1.0.0 LANGUAGES CXX)

add_compile_definitions(UNICODE)
add_library(${PROJECT_NAME} main.cpp)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spdlog
)

add_subdirectory(src)

if (ENABLE_ALL_TEST)
    enable_testing()
    foreach(PLUGIN ${PluginList})
        add_subdirectory(${CMAKE_SOURCE_DIR}/src/${PluginName}/test)
    endforeach()
endif()

