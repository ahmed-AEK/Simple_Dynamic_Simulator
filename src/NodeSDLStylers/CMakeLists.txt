cmake_minimum_required(VERSION 3.11)

add_library(NodeSDLStylers
BlockStyler.hpp BlockStyler.cpp
SpaceScreenTransformer.hpp SpaceScreenTransformer.cpp
)

target_include_directories(NodeSDLStylers PRIVATE ".")
target_include_directories(NodeSDLStylers INTERFACE "..")

target_link_libraries(NodeSDLStylers PUBLIC SDL_Framework NodeModels)
