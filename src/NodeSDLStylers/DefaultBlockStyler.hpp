#pragma once

#include "NodeSDLStylers/BlockStyler.hpp"

namespace node
{
class DefaultBlockStyler : public BlockStyler
{
public:
	void PositionNodes(model::BlockModel& block) override;

	void DrawBlockOutline(SDL_Renderer* renderer, const model::Rect& bounds,
		const SpaceScreenTransformer& transformer, bool selected) override;

	void DrawBlockSocket(SDL_Renderer* renderer, const model::Point& center,
		const SpaceScreenTransformer& transformer, const model::BlockSocketModel::SocketType& type) override;

	void DrawBlockDetails(SDL_Renderer* renderer, const model::Rect& bounds,
		const SpaceScreenTransformer& transformer, bool selected) override;

private:
	RoundRectPainter m_inner_painter;
	RoundRectPainter m_outer_painter;
};
}