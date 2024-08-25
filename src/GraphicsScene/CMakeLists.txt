cmake_minimum_required(VERSION 3.11)

add_library(GraphicsScene 
GraphicsScene.cpp GraphicsScene.hpp
GraphicsObject.cpp GraphicsObject.hpp
BlockObject.cpp BlockObject.hpp
BlockSocketObject.cpp BlockSocketObject.hpp
NetObject.cpp NetObject.hpp

GraphicsScene_exports.h
IGraphicsScene.hpp

tools/GraphicsTool.hpp
tools/ArrowTool.hpp    tools/ArrowTool.cpp
tools/DeleteTool.hpp   tools/DeleteTool.cpp
tools/NetTool.hpp      tools/NetTool.cpp

GraphicsLogic/GraphicsLogic.cpp 	    GraphicsLogic/GraphicsLogic.hpp
GraphicsLogic/NewNetLogic.cpp 	    GraphicsLogic/NewNetLogic.hpp
GraphicsLogic/NewNetJunction.cpp 	    GraphicsLogic/NewNetJunction.hpp
GraphicsLogic/NodeDeleteLogic.cpp 	    GraphicsLogic/NodeDeleteLogic.hpp
GraphicsLogic/ScreenDragLogic.cpp       GraphicsLogic/ScreenDragLogic.hpp
GraphicsLogic/BlockDragLogic.cpp        GraphicsLogic/BlockDragLogic.hpp
GraphicsLogic/LeafNetNodeDragLogic.cpp  GraphicsLogic/LeafNetNodeDragLogic.hpp
GraphicsLogic/VSegmentDragLogic.cpp     GraphicsLogic/VSegmentDragLogic.hpp
GraphicsLogic/SegmentDeleteLogic.cpp    GraphicsLogic/SegmentDeleteLogic.hpp
GraphicsLogic/BlockDeleteLogic.cpp      GraphicsLogic/BlockDeleteLogic.hpp
GraphicsLogic/NetNodeDeleteLogic.cpp    GraphicsLogic/NetNodeDeleteLogic.hpp
ToolsManager.hpp ToolsManager.cpp
ToolButton.hpp ToolButton.cpp
GraphicsObjectsManager.hpp GraphicsObjectsManager.cpp
)

target_include_directories(GraphicsScene PRIVATE ".")
target_include_directories(GraphicsScene INTERFACE "..")

target_compile_definitions(GraphicsScene PRIVATE GRAPHICSSCENE_EXPORTS)

target_link_libraries(GraphicsScene PUBLIC toolgui NodeSDLStylers)
