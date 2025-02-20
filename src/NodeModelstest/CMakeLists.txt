cmake_minimum_required(VERSION 3.11)

add_executable(NodeModelstest 
"NetSegmentModeltest.cpp"
"NetModeltest.cpp"
Blocktest.cpp
"SceneModeltest.cpp"
pch.h
)
target_link_libraries(NodeModelstest PRIVATE GTest::gtest_main NodeModels)
target_include_directories(NodeModelstest PRIVATE '.')

include(GoogleTest)
gtest_discover_tests(NodeModelstest 
WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_precompile_headers(NodeModelstest PRIVATE pch.h)
