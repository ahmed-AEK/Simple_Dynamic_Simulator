#pragma once

#include "SDL_Framework/SDL_headers.h"
#include "NodeModels/BlockModel.hpp"
#include "toolgui/NodeMacros.h"
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"

namespace node
{
	struct BlockStyler
	{
		static constexpr int SocketLength = 15;
		void PositionNodes(model::BlockModel& block);

		void DrawBlock(SDL_Renderer* renderer, const model::BlockModel& block,
			const SpaceScreenTransformer& transformer);

	};
}
