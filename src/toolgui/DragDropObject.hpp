#pragma once
#include <string>
#include "SDL_Framework/SDL_headers.h"
#include "toolgui/NodeMacros.h"
#include "NodeModels/BlockModel.hpp"

namespace node
{
	class BlockStyler;

	struct DragDropObject
	{
		std::string blockTemplateName;
		model::BlockModel block;
		std::shared_ptr<BlockStyler> styler;

		void Draw(SDL_Renderer* renderer, const SDL_FPoint& p) const
		{
			UNUSED_PARAM(renderer);
			UNUSED_PARAM(p);
		}
	};
}