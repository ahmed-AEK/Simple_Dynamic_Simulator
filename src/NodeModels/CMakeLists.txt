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
SubSystemBlocksDataManager.hpp SubSystemBlocksDataManager.cpp
BlockData.hpp
)

target_include_directories(NodeModels PRIVATE ".")
target_include_directories(NodeModels PUBLIC "..")

# file(COPY ../../assets DESTINATION ${CMAKE_BINARY_DIR}/bin)
