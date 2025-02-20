cmake_minimum_required(VERSION 3.11)

add_library(SDL_Framework 
SDLFramework.cpp SDLFramework.hpp
SDLRenderer.cpp SDLRenderer.hpp
SDLCPP.hpp SDL_headers.h
SVGRasterizer.hpp SVGRasterizer.cpp
Utility.hpp Utility.cpp
SDL_Math.hpp
pch.h
)

target_include_directories(SDL_Framework PRIVATE ".")
target_include_directories(SDL_Framework PUBLIC "..")

target_link_libraries(SDL_Framework PUBLIC lunasvg::lunasvg AssetsManager)

if ((DEFINED BUILD_STATIC) AND (${BUILD_STATIC}))
target_link_libraries(SDL_Framework PUBLIC SDL3::SDL3-static SDL3_ttf::SDL3_ttf-static SDL3_image::SDL3_image-static)
else()
target_link_libraries(SDL_Framework PUBLIC SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image)
endif()

target_precompile_headers(SDL_Framework PRIVATE pch.h)
