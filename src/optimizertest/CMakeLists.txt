cmake_minimum_required(VERSION 3.11)


add_executable(NLOptimizertest 
flatmaptest.cpp
EquationTest.cpp
NLEquationTest.cpp
NLSolverTest.cpp
DiffSolverTest.cpp
DiffEquationTest.cpp
NLDiffSolverTest.cpp
FatAnyTest.cpp
NLStatefulEquationTest.cpp
ObserverTest.cpp
)

target_include_directories(NLOptimizertest PRIVATE ".")
target_include_directories(NLOptimizertest PUBLIC "..")

target_link_libraries(NLOptimizertest PUBLIC NLOptimizer)
target_link_libraries(NLOptimizertest PRIVATE GTest::gtest_main toolgui)
cmake_minimum_required(VERSION 3.11)

include(GoogleTest)
gtest_discover_tests(NLOptimizertest 
WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
