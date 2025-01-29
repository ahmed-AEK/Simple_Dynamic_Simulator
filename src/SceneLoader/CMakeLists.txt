cmake_minimum_required(VERSION 3.11)

add_library(SceneLoader 
SceneLoader.hpp
SceneLoader.cpp
SQLSceneLoader.hpp SQLSceneLoader.cpp
BlockLoader.hpp
SQLBlockLoader.hpp SQLBlockLoader.cpp
NetLoader.hpp
SQLNetLoader.hpp SQLNetLoader.cpp
pch.h
)

target_include_directories(SceneLoader PRIVATE ".")
target_include_directories(SceneLoader PUBLIC "..")

target_link_libraries(SceneLoader PUBLIC 
                        SQLiteCpp
				NodeModels
                        )


target_precompile_headers(SceneLoader PRIVATE pch.h)
