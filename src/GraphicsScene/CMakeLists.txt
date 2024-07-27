cmake_minimum_required(VERSION 3.11)

add_library(GraphicsScene 
GraphicsScene.cpp GraphicsScene.hpp
GraphicsObject.cpp GraphicsObject.hpp
DraggableObject.cpp DraggableObject.hpp
BlockObject.cpp BlockObject.hpp
BlockSocketObject.cpp BlockSocketObject.hpp
NetObject.cpp NetObject.hpp

GraphicsScene_exports.h
IGraphicsScene.hpp

tools/GraphicsTool.hpp
tools/ArrowTool.hpp tools/ArrowTool.cpp

GraphicsLogic/GraphicsLogic.cpp 	GraphicsLogic/GraphicsLogic.hpp
GraphicsLogic/NewNet.cpp 		GraphicsLogic/NewNet.hpp
GraphicsLogic/NewNetJunction.cpp 	GraphicsLogic/NewNetJunction.hpp
GraphicsLogic/NodeDeleteLogic.cpp 	GraphicsLogic/NodeDeleteLogic.hpp

ToolsManager.hpp ToolsManager.cpp
ToolButton.hpp ToolButton.cpp
)

target_include_directories(GraphicsScene PRIVATE ".")
target_include_directories(GraphicsScene INTERFACE "..")

target_compile_definitions(GraphicsScene PRIVATE GRAPHICSSCENE_EXPORTS)

target_link_libraries(GraphicsScene PUBLIC toolgui NodeSDLStylers)
