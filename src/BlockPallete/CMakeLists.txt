cmake_minimum_required(VERSION 3.11)

add_library(BlockPallete 
BlockPallete.hpp BlockPallete.cpp
PalleteProvider.hpp PalleteProvider.cpp
)

target_include_directories(BlockPallete PRIVATE ".")
target_include_directories(BlockPallete INTERFACE "..")

target_link_libraries(BlockPallete PUBLIC toolgui)
