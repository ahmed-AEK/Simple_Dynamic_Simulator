cmake_minimum_required(VERSION 3.11)

add_executable(SceneLoadertest
SQLSceneLoadertest.cpp
SQLBlockLoadertest.cpp
SQLNetLoadertest.cpp
pch.h
)

target_link_libraries(SceneLoadertest PRIVATE GTest::gtest_main GTest::gmock_main SceneLoader)
target_include_directories(SceneLoadertest PRIVATE '.')

include(GoogleTest)
gtest_discover_tests(SceneLoadertest
WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_precompile_headers(SceneLoadertest PRIVATE pch.h)
