#pragma once

#include "SDL_Framework/SDL_headers.h"
#include "NodeModels/BlockModel.hpp"
#include "toolgui/NodeMacros.h"
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include "SDL_Framework/Utility.hpp"

namespace node
{

namespace model
{
	struct BlockDataCRef;
}

class BlockStyler
{
public:
	static constexpr int SocketLength = 15;

	virtual void PositionSockets(std::span<model::BlockSocketModel> sockets, const model::Rect& bounds, 
		model::BlockOrientation orientation) const;

	void DrawBlock(SDL_Renderer* renderer, const model::BlockModel& model,
		const SpaceScreenTransformer& transformer, bool selected);

	virtual void DrawBlockOutline(SDL_Renderer* renderer, const model::Rect& bounds,
		const SpaceScreenTransformer& transformer, model::BlockOrientation orientation, bool selected) = 0;

	virtual void DrawBlockSocket(SDL_Renderer* renderer, const model::Point& center,
		const SpaceScreenTransformer& transformer, const model::BlockSocketModel::SocketType& type) = 0;

	virtual void DrawBlockDetails(SDL_Renderer* renderer, const model::Rect& bounds,
		const SpaceScreenTransformer& transformer, model::BlockOrientation orientation, bool selected);

	virtual void UpdateProperties(const model::BlockDataCRef& model);

	virtual ~BlockStyler();
};

}
