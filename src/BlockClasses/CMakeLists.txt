cmake_minimum_required(VERSION 3.11)

add_library(BlockClasses 
BlockClassesManager.cpp BlockClassesManager.hpp
BlockClass.cpp BlockClass.hpp
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
)

target_include_directories(BlockClasses PRIVATE ".")
target_include_directories(BlockClasses PUBLIC "..")
target_link_libraries(BlockClasses PUBLIC NodeModels NLOptimizer toolgui)
