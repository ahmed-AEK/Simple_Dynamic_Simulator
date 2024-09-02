cmake_minimum_required(VERSION 3.11)

add_library(BlockClasses 
BlockClassesManager.cpp BlockClassesManager.hpp
BlockClass.cpp BlockClass.hpp
AddBlockClass.cpp AddBlockClass.hpp
GainBlockClass.cpp GainBlockClass.hpp
ConstantSourceClass.cpp ConstantSourceClass.hpp
ScopeDisplayClass.cpp ScopeDisplayClass.hpp
ScopeDisplayDialog.cpp ScopeDisplayDialog.hpp
)

target_include_directories(BlockClasses PRIVATE ".")
target_include_directories(BlockClasses PUBLIC "..")
target_link_libraries(BlockClasses PUBLIC NodeModels NLOptimizer toolgui)
