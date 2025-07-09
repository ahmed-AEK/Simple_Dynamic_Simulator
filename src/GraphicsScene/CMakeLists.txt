cmake_minimum_required(VERSION 3.11)

add_library(GraphicsScene 
GraphicsScene.cpp GraphicsScene.hpp
GraphicsObject.cpp GraphicsObject.hpp
BlockObject.cpp BlockObject.hpp
BlockSocketObject.cpp BlockSocketObject.hpp
NetObject.cpp NetObject.hpp

GraphicsScene_exports.h

tools/GraphicsTool.hpp
tools/ArrowTool.hpp                     tools/ArrowTool.cpp
tools/DeleteTool.hpp                    tools/DeleteTool.cpp
tools/NetTool.hpp                       tools/NetTool.cpp
tools/GraphicsToolsHandler.hpp          tools/GraphicsToolsHandler.cpp
GraphicsLogic/GraphicsLogic.cpp 	    GraphicsLogic/GraphicsLogic.hpp
GraphicsLogic/NewNetLogic.cpp 	    GraphicsLogic/NewNetLogic.hpp
GraphicsLogic/NewNetJunction.cpp 	    GraphicsLogic/NewNetJunction.hpp
GraphicsLogic/ScreenDragLogic.cpp       GraphicsLogic/ScreenDragLogic.hpp
GraphicsLogic/BlockDragLogic.cpp        GraphicsLogic/BlockDragLogic.hpp
GraphicsLogic/LeafNetNodeDragLogic.cpp  GraphicsLogic/LeafNetNodeDragLogic.hpp
GraphicsLogic/MiddleSegmentDragLogic.cpp     GraphicsLogic/MiddleSegmentDragLogic.hpp
GraphicsLogic/NetDeleteLogic.cpp    GraphicsLogic/NetDeleteLogic.hpp
GraphicsLogic/BlockDeleteLogic.cpp      GraphicsLogic/BlockDeleteLogic.hpp
GraphicsLogic/BlockResizeLogic.cpp      GraphicsLogic/BlockResizeLogic.hpp
GraphicsLogic/BlockRotateLogic.cpp      GraphicsLogic/BlockRotateLogic.hpp
ToolsManager.hpp ToolsManager.cpp
ToolButton.hpp ToolButton.cpp
GraphicsObjectsManager.hpp GraphicsObjectsManager.cpp
BlockResizeObject.hpp BlockResizeObject.cpp
HoverHighlightObject.hpp HoverHighlightObject.cpp
SolverUtils.hpp SolverUtils.cpp
GraphicsLogic/Anchors.hpp GraphicsLogic/Anchors.cpp
GraphicsLogic/TemporaryNetManager.hpp GraphicsLogic/TemporaryNetManager.cpp
NetCategoriesStyleManager.hpp NetCategoriesStyleManager.cpp
pch.h
)

target_include_directories(GraphicsScene PRIVATE ".")
target_include_directories(GraphicsScene INTERFACE "..")

target_compile_definitions(GraphicsScene PRIVATE GRAPHICSSCENE_EXPORTS)

target_link_libraries(GraphicsScene PUBLIC toolgui NodeSDLStylers NetUtils)

target_precompile_headers(GraphicsScene PRIVATE pch.h)
