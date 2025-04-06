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
DialogControls.hpp DialogControls.cpp
ToolTipWidget.hpp ToolTipWidget.cpp
StackedWidget.hpp StackedWidget.cpp
TabbedView.hpp TabbedView.cpp
ToolTipWidgetMixin.hpp ToolTipWidgetMixin_impl.hpp
SceneGrid.hpp SceneGrid.cpp
ScrollView.hpp ScrollView.cpp
LogView.hpp LogView.cpp
pch.h
)

target_include_directories(toolgui PRIVATE ".")
target_include_directories(toolgui INTERFACE "..")

target_compile_definitions(toolgui PRIVATE TOOLGUI_EXPORTS)

target_link_libraries(toolgui PUBLIC 
                        SDL_Framework NodeModels 
                        )

target_precompile_headers(toolgui PRIVATE pch.h)

