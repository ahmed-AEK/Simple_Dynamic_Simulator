
#include "toolgui/Widget.hpp"
#include "toolgui/NodeMacros.h"
#include "toolgui/Scene.hpp"
#include "toolgui/Application.hpp"

#include "NodeModels/Observer.hpp"
#include "NodeModels/BlockModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "NodeModels/BlockData.hpp"

#include "SDL_Framework/SDL_headers.h"
#include "SDL_Framework/SDLCPP.hpp"
#include "SDL_Framework/SDLRenderer.hpp"

#include "NodeSDLStylers/BlockStyler.hpp"
#include "NodeSDLStylers/BlockStylerFactory.hpp"

#include "PluginAPI/BlockClass.hpp"
#include "PluginAPI/BlockClassesManager.hpp"
#include "PluginAPI/Logger.hpp"

#include <cassert>
#include <algorithm>
#include <iterator>
