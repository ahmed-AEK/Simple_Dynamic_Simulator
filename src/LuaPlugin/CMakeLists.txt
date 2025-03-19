cmake_minimum_required(VERSION 3.11)

add_library(LuaPlugin 
LuaRuntime.hpp LuaRuntime.cpp
LuaExpressionClass.hpp LuaExpressionClass.cpp
LuaStatefulEqnClass.hpp LuaStatefulEqnClass.cpp
pch.h
)

target_include_directories(LuaPlugin PRIVATE ".")
target_include_directories(LuaPlugin PUBLIC "..")
target_link_libraries(LuaPlugin PUBLIC NodeModels NLOptimizer toolgui PluginAPI)
target_link_libraries(LuaPlugin PRIVATE lua sol2)
target_precompile_headers(LuaPlugin PRIVATE pch.h)
