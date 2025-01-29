cmake_minimum_required(VERSION 3.11)

add_executable(toolguitest 
NetSegmenttest.cpp
BlockObjecttest.cpp
pch.h
)
target_link_libraries(toolguitest PRIVATE GTest::gtest_main GTest::gmock_main toolgui GraphicsScene)
target_include_directories(toolguitest PRIVATE '.')

include(GoogleTest)
gtest_discover_tests(toolguitest
WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_precompile_headers(toolguitest PRIVATE pch.h)
