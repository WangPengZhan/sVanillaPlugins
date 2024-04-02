cmake_minimum_required(VERSION 3.15)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
set(VCPKG_APPLOCAL_DEPS ON)

project(sVanillaPlugins VERSION 1.0.0 LANGUAGES CXX)

add_compile_definitions(UNICODE)

add_subdirectory(TemplatePlugin)

option(ENABLE_TEST "Enable Test" OFF)
if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

