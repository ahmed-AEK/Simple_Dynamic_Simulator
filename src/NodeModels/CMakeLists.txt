cmake_minimum_required(VERSION 3.11)

add_library(NodeModels 
BlockModel.cpp BlockModel.hpp
Utils.hpp
NodeScene.cpp NodeScene.hpp
NetModel.cpp NetModel.hpp
Observer.hpp
SceneModelManager.hpp SceneModelManager.cpp
IdTypes.hpp
FunctionalBlocksDataManager.hpp FunctionalBlocksDataManager.cpp
SubsystemBlocksDataManager.hpp SubsystemBlocksDataManager.cpp
BlockData.hpp
PortBlocksDataManager.hpp PortBlocksDataManager.cpp
BlockPortsUpdate.hpp
pch.h
)

target_include_directories(NodeModels PRIVATE ".")
target_include_directories(NodeModels PUBLIC "..")

target_link_libraries(NodeModels PUBLIC Boost::container_hash Boost::charconv)
# file(COPY ../../assets DESTINATION ${CMAKE_BINARY_DIR}/bin)

target_precompile_headers(NodeModels PRIVATE pch.h)
