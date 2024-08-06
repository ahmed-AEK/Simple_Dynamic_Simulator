#pragma once

#include "SDL_Framework/SDL_headers.h"
#include "NodeModels/BlockModel.hpp"
#include "toolgui/NodeMacros.h"
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"

namespace node
{
	class BlockStyler
	{
	public:
		static constexpr int SocketLength = 15;
		void PositionNodes(model::BlockModel& block);

		void DrawBlock(SDL_Renderer* renderer, const model::BlockModel& model,
			const SpaceScreenTransformer& transformer, bool selected);

		void DrawBlockOutline(SDL_Renderer* renderer, const model::Rect& bounds,
			const SpaceScreenTransformer& transformer, bool selected);

		void DrawBlockSocket(SDL_Renderer* renderer, const model::Point& center,
			const SpaceScreenTransformer& transformer, const model::BlockSocketModel::SocketType& type);

	};
}
