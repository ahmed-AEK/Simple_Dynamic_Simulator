cmake_minimum_required(VERSION 3.11)

add_library(PluginAPI 
BlockClass.cpp BlockClass.hpp
BlockDialog.hpp BlockDialog.cpp
BlockClassesManager.cpp BlockClassesManager.hpp
BlocksPlugin.cpp BlocksPlugin.hpp
BlockTemplate.h TestTemplates.c
BlockCClass.h
BlockClassHelpers.hpp
PluginRuntime.hpp PluginRuntime.cpp
)

target_include_directories(PluginAPI PRIVATE ".")
target_include_directories(PluginAPI PUBLIC "..")
target_link_libraries(PluginAPI PUBLIC NodeModels NLOptimizer)
