cmake_minimum_required(VERSION 3.11)

add_library(NetUtils
NetsSolver.hpp NetsSolver.cpp
Utils.hpp Utils.cpp
)

target_include_directories(NetUtils PRIVATE ".")
target_include_directories(NetUtils PUBLIC "..")

target_link_libraries(NetUtils PUBLIC 
				NodeModels
                        )
