cmake_minimum_required(VERSION 3.11)

add_library(PluginAPI 
BlockClass.cpp BlockClass.hpp
BlockDialog.hpp BlockDialog.cpp
BlockClassesManager.cpp BlockClassesManager.hpp
BlocksPlugin.cpp BlocksPlugin.hpp
BlockTemplate.h
BlockCClass.h
BlockClassHelpers.hpp
PluginRuntime.hpp PluginRuntime.cpp
Logger.hpp 
)

target_include_directories(PluginAPI PRIVATE ".")
target_include_directories(PluginAPI PUBLIC "..")
target_link_libraries(PluginAPI PUBLIC NodeModels NLOptimizer tl::expected)
target_precompile_headers(PluginAPI PRIVATE pch.h)
