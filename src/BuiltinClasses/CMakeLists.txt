cmake_minimum_required(VERSION 3.11)

add_library(BuiltinClasses 
AddBlockClass.cpp AddBlockClass.hpp
GainBlockClass.cpp GainBlockClass.hpp
ConstantSourceClass.cpp ConstantSourceClass.hpp
RampSourceClass.cpp RampSourceClass.hpp
ScopeDisplayClass.cpp ScopeDisplayClass.hpp
ScopeDisplayDialog.cpp ScopeDisplayDialog.hpp
IntegrationBlockClass.cpp IntegrationBlockClass.hpp
DerivativeBlockClass.cpp DerivativeBlockClass.hpp
AddSimpleClass.cpp AddSimpleClass.hpp
MultiplyBlockClass.cpp MultiplyBlockClass.hpp
SineSourceClass.hpp SineSourceClass.cpp
StepSourceClass.hpp StepSourceClass.cpp
ComparatorClass.hpp ComparatorClass.cpp
BuiltinBasicClass.hpp BuiltinBasicClass.cpp
BuiltinClassesPlugin.hpp BuiltinClassesPlugin.cpp
electrical/ElectricalResistorBlockClass.hpp electrical/ElectricalResistorBlockClass.cpp
electrical/ElectricalVDCBlockClass.hpp electrical/ElectricalVDCBlockClass.cpp
pch.h
)

target_include_directories(BuiltinClasses PRIVATE ".")
target_include_directories(BuiltinClasses PUBLIC "..")
target_link_libraries(BuiltinClasses PUBLIC NodeModels NLOptimizer toolgui PluginAPI)

target_precompile_headers(BuiltinClasses PRIVATE pch.h)
