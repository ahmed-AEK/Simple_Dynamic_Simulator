cmake_minimum_required(VERSION 3.19)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # LTO
add_executable(cmaketest main.cpp)

target_link_libraries(cmaketest PUBLIC
        SQLiteCpp
        )