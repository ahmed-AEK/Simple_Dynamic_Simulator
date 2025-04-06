
#include "toolgui/Widget.hpp"
#include "toolgui/MouseInteractable_interface.hpp"
#include "toolgui/NodeMacros.h"
#include "toolgui/Scene.hpp"
#include "toolgui/ContextMenu.hpp"
#include "toolgui/ToolBar.hpp"

#include "NetUtils/NetsSolver.hpp"
#include "NetUtils/Utils.hpp"

#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include "NodeSDLStylers/BlockStyler.hpp"
#include "NodeSDLStylers/BlockStylerFactory.hpp"

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/BlockData.hpp"
#include "NodeModels/SceneModelManager.hpp"

#include "SDL_Framework/SDLRenderer.hpp"
#include "SDL_Framework/SDL_headers.h"

#include "PluginAPI/Logger.hpp"

#include <cassert>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <array>
#include <stack>
