cmake_minimum_required(VERSION 3.11)

add_executable(NetUtilstest
NetsSolvertest.cpp
)

target_link_libraries(NetUtilstest PRIVATE GTest::gmock_main GTest::gmock NetUtils)
target_include_directories(NetUtilstest PRIVATE '.')

include(GoogleTest)
gtest_discover_tests(NetUtilstest 
WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)