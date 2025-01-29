cmake_minimum_required(VERSION 3.11)

add_library(NodeSDLStylers
BlockStyler.hpp BlockStyler.cpp
SpaceScreenTransformer.hpp SpaceScreenTransformer.cpp
BlockStylerFactory.hpp BlockStylerFactory.cpp
DefaultBlockStyler.hpp DefaultBlockStyler.cpp
GainBlockStyler.hpp GainBlockStyler.cpp
TextBlockStyler.hpp TextBlockStyler.cpp
PropertyPrintStyler.hpp PropertyPrintStyler.cpp
SVGBlockStyler.hpp SVGBlockStyler.cpp
pch.h
)

target_include_directories(NodeSDLStylers PRIVATE ".")
target_include_directories(NodeSDLStylers INTERFACE "..")

target_link_libraries(NodeSDLStylers PUBLIC SDL_Framework NodeModels)

target_precompile_headers(NodeSDLStylers PRIVATE pch.h)
