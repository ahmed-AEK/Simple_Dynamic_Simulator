cmake_minimum_required(VERSION 3.11)

add_library(NodePallete 
NodePallete.hpp NodePallete.cpp
)

target_include_directories(NodePallete PRIVATE ".")
target_include_directories(NodePallete INTERFACE "..")

target_link_libraries(NodePallete PUBLIC toolgui)
