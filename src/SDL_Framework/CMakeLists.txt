cmake_minimum_required(VERSION 3.11)

add_library(SDL_Framework 
SDLFramework.cpp SDLFramework.hpp
SDLRenderer.cpp SDLRenderer.hpp
SDLCPP.hpp SDL_headers.h
SVGRasterizer.hpp SVGRasterizer.cpp
Utility.hpp
Utility.cpp
)

target_include_directories(SDL_Framework PRIVATE ".")
target_include_directories(SDL_Framework PUBLIC "..")

target_link_libraries(SDL_Framework PUBLIC SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image lunasvg::lunasvg AssetsManager)
