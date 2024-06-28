cmake_minimum_required(VERSION 3.11)

add_library(SceneLoader 
SceneLoader.hpp
SQLSceneLoader.hpp
SQLSceneLoader.cpp
NodeLoader.hpp
SQLNodeLoader.hpp
SQLNodeLoader.cpp
)

target_include_directories(SceneLoader PRIVATE ".")
target_include_directories(SceneLoader PUBLIC "..")

target_link_libraries(SceneLoader PUBLIC 
                        SQLiteCpp
                        )
