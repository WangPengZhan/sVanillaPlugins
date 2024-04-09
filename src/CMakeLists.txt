
set(DIRECTORIES)
if (SINGLE_PLUGIN_BUILD)
    if (NOT SINGLE_PLUGIN_NAME)
        message(FATAL_ERROR "Single Plugin Name not set: SINGLE_PLUGIN_NAME")
    else ()
        list(APPEND DIRECTORIES "${SINGLE_PLUGIN_NAME}")
    endif ()
else ()
    file(GLOB DIRECTORIES LIST_DIRECTORIES true "*")
endif ()

foreach (DIR ${DIRECTORIES})
    if (IS_DIRECTORY "${DIR}" AND EXISTS "${DIR}/CMakeLists.txt")
        get_filename_component(PluginName "${DIR}" NAME)
        add_subdirectory(${DIR})
        message(STATUS "Plugin ${PluginName} added")
        list(APPEND PluginList ${PluginName})
    endif ()
endforeach ()

add_dependencies(${PROJECT_NAME} ${PluginList})

set_target_properties(${PROJECT_NAME} PROPERTIES _PLUGINS_ "${PLUGINS}")
