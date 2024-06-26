cmake_minimum_required(VERSION 3.15)

project(TemplatePlugin VERSION 1.0.0 LANGUAGES CXX)

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
find_package(spdlog REQUIRED)

# 如果OpenSSL被找到，包含OpenSSL的头文件目录
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        nlohmann_json::nlohmann_json
        CURL::libcurl
        spdlog::spdlog
        ${OPENSSL_LIBRARIES}
)


# install
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
