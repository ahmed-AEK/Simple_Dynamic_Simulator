cmake_minimum_required(VERSION 3.11)


add_library(NLOptimizer 
flatmap.hpp
Equation.hpp
NLEquation.cpp NLEquation.hpp
DiffEquation.cpp DiffEquation.hpp
DiffSolver.cpp DiffSolver.hpp
NLDiffSolver.cpp NLDiffSolver.hpp
FatAny.cpp FatAny.hpp
NLStatefulEquation.cpp NLStatefulEquation.hpp
Observer.cpp Observer.hpp
SourceEq.cpp SourceEq.hpp
NLGraphSolver.cpp NLGraphSolver.hpp
DiffSolver_private.hpp
NLGraphSolver_private.hpp
FlowEquation.cpp FlowEquation.hpp
PotentialEquation.cpp PotentialEquation.hpp
pch.h
)

target_include_directories(NLOptimizer PRIVATE ".")
target_include_directories(NLOptimizer PUBLIC "..")

target_link_libraries(NLOptimizer PUBLIC NLopt::nlopt Boost::numeric_odeint)

target_precompile_headers(NLOptimizer PRIVATE pch.h)
