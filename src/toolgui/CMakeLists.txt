cmake_minimum_required(VERSION 3.11)

add_library(toolgui 
MouseInteractable_interface.hpp MouseInteractable.hpp
Application.cpp Application.hpp
Widget.cpp Widget.hpp
Scene.cpp Scene.hpp
DraggableWidget.cpp DraggableWidget.hpp
ContextMenu.cpp ContextMenu.hpp
Handle.hpp
ButtonWidget.cpp ButtonWidget.hpp
NodeMacros.h
SidePanel.hpp SidePanel.cpp
DragDropObject.hpp DragDropObject.cpp
ToolBar.hpp ToolBar.cpp
Dialog.hpp Dialog.cpp
MessageBox.hpp MessageBox.cpp
LineEditControl.hpp LineEditControl.cpp
)

target_include_directories(toolgui PRIVATE ".")
target_include_directories(toolgui INTERFACE "..")

target_compile_definitions(toolgui PRIVATE TOOLGUI_EXPORTS)

target_link_libraries(toolgui PUBLIC 
                        SDL_Framework NodeModels 
                        )
