cmake_minimum_required(VERSION 3.11)

add_executable(DynamicSimulator main.cpp 
NodeEditorApp.cpp NodeEditorApp.hpp
MainNodeScene.cpp MainNodeScene.hpp
ExampleContextMenu.cpp ExampleContextMenu.hpp
NodeGraphicsScene.cpp NodeGraphicsScene.hpp
SimulatorRunner.hpp SimulatorRunner.cpp
BlockPropertiesDialog.hpp BlockPropertiesDialog.cpp
SimulationSettingsDialog.hpp SimulationSettingsDialog.cpp
SimulationManager.hpp SimulationManager.cpp
NewSceneDialog.hpp NewSceneDialog.cpp
SceneManager.hpp SceneManager.cpp
IdTypes.hpp
)

target_include_directories(DynamicSimulator PRIVATE ".")
target_include_directories(DynamicSimulator INTERFACE "..")

target_link_libraries(DynamicSimulator PRIVATE 
toolgui GraphicsScene BlockPallete BlockClasses SceneLoader)

# file(COPY ../../assets DESTINATION ${CMAKE_BINARY_DIR}/bin)
