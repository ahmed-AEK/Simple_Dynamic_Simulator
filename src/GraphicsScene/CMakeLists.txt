cmake_minimum_required(VERSION 3.11)

project(node_editor)

add_library(GraphicsScene 
GraphicsScene.cpp GraphicsScene.hpp
GraphicsObject.cpp GraphicsObject.hpp
DraggableObject.cpp DraggableObject.hpp
Node.cpp Node.hpp
NodeSocket.cpp NodeSocket.hpp
NetObject.cpp NetObject.hpp

GraphicsScene_exports.h
SpaceScreenTransformer.cpp SpaceScreenTransformer.hpp
IGraphicsScene.hpp
IGraphicsSceneController.hpp
GraphicsSceneController.cpp GraphicsSceneController.hpp

GraphicsLogic/GraphicsLogic.cpp 	GraphicsLogic/GraphicsLogic.hpp
GraphicsLogic/NewNet.cpp 		GraphicsLogic/NewNet.hpp
GraphicsLogic/NewNetJunction.cpp 	GraphicsLogic/NewNetJunction.hpp
GraphicsLogic/NodeDeleteLogic.cpp 	GraphicsLogic/NodeDeleteLogic.hpp

)

target_include_directories(GraphicsScene PRIVATE ".")
target_include_directories(GraphicsScene INTERFACE "..")

target_compile_definitions(GraphicsScene PRIVATE GRAPHICSSCENE_EXPORTS)

target_link_libraries(GraphicsScene PUBLIC toolgui)
