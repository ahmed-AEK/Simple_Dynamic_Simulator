cmake_minimum_required(VERSION 3.11)


add_executable(Optimizer_Perf
main.cpp
)

target_include_directories(Optimizer_Perf PRIVATE ".")
target_include_directories(Optimizer_Perf PUBLIC "..")

target_link_libraries(Optimizer_Perf PRIVATE NLOptimizer)
