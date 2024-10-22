cmake_minimum_required(VERSION 3.11)

add_library(AssetsManager
AssetStruct.hpp
AssetsManager.hpp AssetsManager.cpp
)

include("./data/CMakeInclude.txt")

target_include_directories(AssetsManager PRIVATE 
"."
"./data"
)

target_include_directories(AssetsManager INTERFACE "..")
