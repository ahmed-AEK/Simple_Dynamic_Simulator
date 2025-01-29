

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"

#include "optimizer/NLEquation.hpp"
#include "optimizer/DiffEquation.hpp"
#include "optimizer/NLStatefulEquation.hpp"
#include "optimizer/Observer.hpp"
#include "optimizer/SourceEq.hpp"

#include "toolgui/NodeMacros.h"
#include "toolgui/Widget.hpp"
#include "toolgui/Dialog.hpp"

#include "SDL_Framework/SDL_headers.h"

#include <cmath>
#include <algorithm>
#include <charconv>
#include <cstring>
#include <array>
#include <numbers>
