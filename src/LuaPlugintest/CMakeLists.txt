cmake_minimum_required(VERSION 3.11)

add_executable(LuaPlugintest 
StatefulEqntest.cpp
pch.h
)

target_link_libraries(LuaPlugintest PRIVATE GTest::gtest_main LuaPlugin)
target_include_directories(LuaPlugintest PRIVATE '.')

include(GoogleTest)
gtest_discover_tests(LuaPlugintest 
WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_precompile_headers(LuaPlugintest PRIVATE pch.h)
