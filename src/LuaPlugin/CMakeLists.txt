cmake_minimum_required(VERSION 3.11)

add_library(LuaPlugin 
LuaRuntime.hpp LuaRuntime.cpp
LuaExpressionClass.hpp LuaExpressionClass.cpp
LuaStatefulEqnClass.hpp LuaStatefulEqnClass.cpp
LuaStandaloneStatefulEqnClass.hpp LuaStandaloneStatefulEqnClass.cpp
LuaStatefulEqn.hpp LuaStatefulEqn.cpp
LuaEditorDialog.hpp LuaEditorDialog.cpp
pch.h
)

target_include_directories(LuaPlugin PRIVATE ".")
target_include_directories(LuaPlugin PUBLIC "..")
target_include_directories(LuaPlugin PRIVATE ${SOL2_INCLUDE_DIRS})

target_link_libraries(LuaPlugin PUBLIC NodeModels NLOptimizer toolgui PluginAPI)
target_link_libraries(LuaPlugin PRIVATE lua)
target_precompile_headers(LuaPlugin PRIVATE pch.h)
