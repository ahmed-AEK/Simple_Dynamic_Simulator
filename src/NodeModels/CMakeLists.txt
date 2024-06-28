cmake_minimum_required(VERSION 3.11)

add_library(NodeModels 
NodeModel.cpp NodeModel.hpp
Utils.hpp
NodeScene.cpp NodeScene.hpp
NetModel.cpp NetModel.hpp
)

target_include_directories(NodeModels PRIVATE ".")
target_include_directories(NodeModels PUBLIC "..")

# file(COPY ../../assets DESTINATION ${CMAKE_BINARY_DIR}/bin)
