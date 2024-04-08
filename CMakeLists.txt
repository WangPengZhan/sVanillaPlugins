cmake_minimum_required(VERSION 3.15)

project(sVanillaPlugins VERSION 1.0.0 LANGUAGES CXX)

add_compile_definitions(UNICODE)
add_library(${PROJECT_NAME} main.cpp)

set(PLUGINS)

set(GIT_MODULES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/.gitmodules")
file(READ "${GIT_MODULES_FILE}" GITMODULES_CONTENT)
string(REGEX MATCHALL "\\[submodule \"([^\"]+)\"\\]" SUBMODULE_SECTIONS "${GITMODULES_CONTENT}")
foreach(SECTION ${SUBMODULE_SECTIONS})
    string(REGEX MATCH "\\[submodule \"([^\"]+)\"\\]" _ ${SECTION})
    string(FIND "${CMAKE_MATCH_1}" "/" last_slash_pos REVERSE)
    if(last_slash_pos GREATER -1)
        math(EXPR last_slash_pos "${last_slash_pos} + 1")
        string(SUBSTRING "${CMAKE_MATCH_1}" ${last_slash_pos} -1 ${CMAKE_MATCH_1})
    endif()
    list(APPEND PLUGINS ${CMAKE_MATCH_1})
endforeach()

include(ExternalProject)
if (PLUGINS)
    foreach(PLUGIN ${PLUGINS})
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN})
    endforeach()
    add_dependencies(${PROJECT_NAME} ${PLUGINS})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES _PLUGINS_ "${PLUGINS}")

option(ENABLE_TEST "Enable Test" OFF)
if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

