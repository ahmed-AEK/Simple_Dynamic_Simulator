#pragma once
#include <string>
#include "SDL_Framework/SDL_headers.h"
#include "toolgui/NodeMacros.h"
#include "NodeModels/BlockModel.hpp"

namespace node
{
	struct DragDropObject
	{
		std::string blockTemplateName;
		model::BlockModel block;
		void Draw(SDL_Renderer* renderer, const SDL_Point& p) const
		{
			UNUSED_PARAM(renderer);
			UNUSED_PARAM(p);
		}
	};
}