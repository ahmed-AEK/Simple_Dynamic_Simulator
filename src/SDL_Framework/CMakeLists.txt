cmake_minimum_required(VERSION 3.11)

add_library(SDL_Framework 
SDLFramework.cpp SDLFramework.hpp
SDLRenderer.cpp SDLRenderer.hpp
SDLCPP.hpp SDL_headers.h

Utility.hpp
Utility.cpp
)

target_include_directories(SDL_Framework PRIVATE ".")
target_include_directories(SDL_Framework PUBLIC "..")

target_link_libraries(SDL_Framework PUBLIC SDL2::SDL2 SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)