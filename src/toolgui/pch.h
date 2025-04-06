#pragma once

#include "AssetsManager/AssetsManager.hpp"

#include "SDL_Framework/SDL_headers.h"
#include "SDL_Framework/Utility.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include "SDL_Framework/SDL_Math.hpp"
#include "SDL_Framework/SDLRenderer.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/BlockData.hpp"
#include "NodeModels/NetModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "NodeModels/SceneModelManager.hpp"
#include "NodeModels/SubsystemBlocksDataManager.hpp"
#include "NodeModels/PortBlocksDataManager.hpp"

#include "PluginAPI/Logger.hpp"

#include <cmath>
#include <ranges>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <fstream>
#include <utility>
#include <atomic>
#include <memory_resource>
#include <memory>