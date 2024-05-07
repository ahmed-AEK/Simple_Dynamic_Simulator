cmake_minimum_required(VERSION 3.11)

project(toolgui)

add_library(toolgui 
MouseInteractable.cpp MouseInteractable.hpp
Application.cpp Application.hpp
Widget.cpp Widget.hpp
Scene.cpp Scene.hpp
DraggableWidget.cpp DraggableWidget.hpp
GraphicsScene.cpp GraphicsScene.hpp
GraphicsObject.cpp GraphicsObject.hpp
DraggableObject.cpp DraggableObject.hpp
ContextMenu.cpp ContextMenu.hpp
Node.cpp Node.hpp
NodeSocket.cpp NodeSocket.hpp
NetObject.cpp NetObject.hpp
NewNet.cpp NewNet.hpp
NewNetJunction.cpp NewNetJunction.hpp
Handle.hpp
ButtonWidget.cpp ButtonWidget.hpp
)

target_include_directories(toolgui PRIVATE ".")
target_include_directories(toolgui INTERFACE "..")

target_compile_definitions(toolgui PRIVATE TOOLGUI_EXPORTS)

target_link_libraries(toolgui PUBLIC 
                        SDL_Framework
                        )
