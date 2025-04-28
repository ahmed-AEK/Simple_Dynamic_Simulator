
//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 


#include "sol/sol.hpp"

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"

#include "PluginAPI/BlockClass.hpp"
#include "PluginAPI/Logger.hpp"

#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"

#include <cmath>
#include <algorithm>
#include <charconv>
#include <cstring>
#include <array>
#include <numbers>
#include <format>
