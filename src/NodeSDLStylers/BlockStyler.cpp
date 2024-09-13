#include "BlockStyler.hpp"

void node::BlockStyler::DrawBlock(SDL_Renderer* renderer, const model::BlockModel& model, const SpaceScreenTransformer& transformer, bool selected)
{
	auto&& bounds = model.GetBounds();
	DrawBlockOutline(renderer, bounds, transformer, selected);

	for (const auto& socket : model.GetSockets())
	{
		DrawBlockSocket(renderer, socket.GetPosition(), transformer, socket.GetType());
	}

	DrawBlockDetails(renderer, bounds, transformer, selected);
}
