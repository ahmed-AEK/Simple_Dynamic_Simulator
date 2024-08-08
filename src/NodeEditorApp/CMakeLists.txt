cmake_minimum_required(VERSION 3.11)

add_executable(NodeEditor main.cpp 
NodeEditorApp.cpp NodeEditorApp.hpp
MainNodeScene.cpp MainNodeScene.hpp
ExampleContextMenu.cpp ExampleContextMenu.hpp
NodeGraphicsScene.cpp NodeGraphicsScene.hpp
)

target_include_directories(NodeEditor PRIVATE ".")
target_include_directories(NodeEditor INTERFACE "..")

target_link_libraries(NodeEditor PRIVATE toolgui GraphicsScene BlockPallete)

# file(COPY ../../assets DESTINATION ${CMAKE_BINARY_DIR}/bin)
