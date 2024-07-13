#pragma once
#include <string>
#include "SDL_Framework/SDL_headers.h"
#include "toolgui/NodeMacros.h"

namespace node
{
	struct DragDropObject
	{
		std::string blockTemplateName;
		void Draw(SDL_Renderer* renderer, const SDL_Point& p) const
		{
			UNUSED_PARAM(renderer);
			UNUSED_PARAM(p);
		}
	};
}