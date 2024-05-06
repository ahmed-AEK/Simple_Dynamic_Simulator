cmake_minimum_required(VERSION 3.11)

project(guitest)

add_executable(toolguitest NetSegmenttest.cpp)
target_link_libraries(toolguitest PRIVATE GTest::gtest_main toolgui)
target_include_directories(toolguitest PRIVATE '.')

include(GoogleTest)
gtest_discover_tests(toolguitest
WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)