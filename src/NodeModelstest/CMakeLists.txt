cmake_minimum_required(VERSION 3.11)

add_executable(NodeModelstest 
NetSegmenttest.cpp
Nettest.cpp
Blocktest.cpp
Scenetest.cpp
pch.h
)
target_link_libraries(NodeModelstest PRIVATE GTest::gtest_main GTest::gmock_main NodeModels)
target_include_directories(NodeModelstest PRIVATE '.')

include(GoogleTest)
gtest_discover_tests(NodeModelstest 
WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_precompile_headers(NodeModelstest PRIVATE pch.h)
