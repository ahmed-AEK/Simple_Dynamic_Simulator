
#include "gtest/gtest.h"

#include "optimizer/DiffSolver.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/Equation.hpp"
#include "optimizer/FatAny.hpp"
#include "optimizer/flatmap.hpp"
#include "optimizer/NLDiffSolver.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"
#include "optimizer/NLEquation.hpp"
#include "optimizer/NLGraphSolver.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/FlowEquation.hpp"
#include "optimizer/PotentialEquation.hpp"

#include "toolgui/NodeMacros.h"

#include <array>
#include <memory>
#include <cmath>
#include <numbers>
#include <memory_resource>