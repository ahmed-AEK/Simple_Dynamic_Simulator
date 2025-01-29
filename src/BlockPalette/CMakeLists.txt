cmake_minimum_required(VERSION 3.11)

add_library(BlockPalette 
BlockPalette.hpp BlockPalette.cpp
PaletteProvider.hpp PaletteProvider.cpp
PaletteCategoryPicker.hpp PaletteCategoryPicker.cpp
PaletteBlocksViewer.hpp PaletteBlocksViewer.cpp
pch.h
)

target_include_directories(BlockPalette PRIVATE ".")
target_include_directories(BlockPalette INTERFACE "..")

target_link_libraries(BlockPalette PUBLIC toolgui)

target_precompile_headers(BlockPalette PRIVATE pch.h)
