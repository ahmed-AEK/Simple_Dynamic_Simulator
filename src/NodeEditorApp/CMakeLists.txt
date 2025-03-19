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
AboutDialog.hpp AboutDialog.cpp
PluginsManager.hpp PluginsManager.cpp
IdTypes.hpp
pch.h
)

target_include_directories(DynamicSimulator PRIVATE ".")
target_include_directories(DynamicSimulator INTERFACE "..")

target_compile_definitions(DynamicSimulator 
PRIVATE APP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
PRIVATE APP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
PRIVATE APP_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

target_link_libraries(DynamicSimulator PRIVATE 
toolgui GraphicsScene BlockPalette BuiltinClasses LuaPlugin PluginAPI SceneLoader)

# file(COPY ../../assets DESTINATION ${CMAKE_BINARY_DIR}/bin)

target_precompile_headers(DynamicSimulator PRIVATE pch.h)
