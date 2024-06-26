cmake_minimum_required(VERSION 3.15)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
set(VCPKG_APPLOCAL_DEPS ON)

project(sVanillaPlugins VERSION 1.0.0 LANGUAGES CXX)

add_compile_definitions(UNICODE)
add_library(${PROJECT_NAME} main.cpp)

set(PLUGINS)

file(GLOB FOLDER_CHILDREN RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_CURRENT_SOURCE_DIR}/*)
if (FOLDER_CHILDREN)
    foreach (CHILD ${FOLDER_CHILDREN})
        if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${CHILD}")
            if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CHILD}/CMakeLists.txt")
                list(APPEND PLUGINS ${CHILD})
            endif ()
        endif ()
    endforeach ()
endif ()

if (PLUGINS)
    foreach(plugin ${PLUGINS})
        add_subdirectory("${plugin}")
    endforeach()
    add_dependencies(${PROJECT_NAME} ${PLUGINS})
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES _PLUGINS_ "${PLUGINS}")

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)

option(ENABLE_TEST "Enable Test" OFF)
if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

