function(EnableTest PluginName)
    include(GoogleTest)
    add_subdirectory(
            ${CMAKE_SOURCE_DIR}/ThirdParty/googletest gtest
    )
    set(TestCaseName ${PluginName}Test)
    set(TestDir ${CMAKE_SOURCE_DIR}/src/${PluginName}/test)
    set(SourceDir ${CMAKE_SOURCE_DIR}/src/${PluginName}/src)
endfunction()

function(AddSourceFiles)
    target_link_libraries(
            ${TestCaseName}
            PRIVATE
            gtest_main
            gmock_main
    )
    file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)
    file(GLOB_RECURSE
            PLUGIN_SOURCES
            LIST_DIRECTORIES false
            ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp
            ${CMAKE_SOURCE_DIR}/include/*.h
    )
    set(SOURCES ${TEST_SOURCES} ${PLUGIN_SOURCES})
    target_sources(${TestCaseName} PUBLIC ${SOURCES})
    include_directories(${SourceDir})
endfunction()


function(AddLibrary ADDITIONAL_LIBRARIES)
    target_link_libraries(
            ${TestCaseName}
            PRIVATE
            ${ADDITIONAL_LIBRARIES}
    )
endfunction()