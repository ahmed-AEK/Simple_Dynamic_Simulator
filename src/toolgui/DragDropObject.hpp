#pragma once
#include <string>
#include "SDL_Framework/SDL_headers.h"
#include "SDL_Framework/SDLRenderer.hpp"
#include "toolgui/NodeMacros.h"
#include "NodeModels/BlockModel.hpp"
#include "NodeModels/BlockData.hpp"

namespace node
{
	class BlockStyler;

	struct DragDropObject
	{
		std::string blockTemplateName;
		model::BlockModel block;
		std::shared_ptr<BlockStyler> styler;
		model::BlockData data;

		void Draw(SDL::Renderer& renderer, const SDL_FPoint& p) const
		{
			UNUSED_PARAM(renderer);
			UNUSED_PARAM(p);
		}
	};
}