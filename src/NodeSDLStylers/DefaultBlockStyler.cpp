#include "DefaultBlockStyler.hpp"
#include <algorithm>
#include "SDL_Framework/Utility.hpp"

void node::DefaultBlockStyler::DrawBlockOutline(SDL::Renderer& renderer, const model::Rect& bounds, const SpaceScreenTransformer& transformer,
	model::BlockOrientation orientation, bool selected)
{
	UNUSED_PARAM(orientation);
	SDL_FRect screenRect = transformer.SpaceToScreenRect(bounds);
	SDL_Color outer_color = selected ?
		renderer.GetColor(ColorRole::block_outline_selected) : renderer.GetColor(ColorRole::block_outline);
	SDL_Color inner_color = renderer.GetColor(ColorRole::block_background);

	ThickFilledRoundRect(renderer, screenRect, static_cast<int>(screenRect.w / 10), 2, outer_color, inner_color,
		m_outer_painter, m_inner_painter);
}

void node::DefaultBlockStyler::DrawBlockSocket(SDL::Renderer& renderer, const model::Point& center, const SpaceScreenTransformer& transformer,
	const model::BlockSocketModel::SocketType& type)
{
	SDL_FPoint socket_length = transformer.SpaceToScreenVector({ SocketLength, SocketLength });
	switch (type)
	{
	case model::BlockSocketModel::SocketType::input:
	{
		const SDL_Color color = renderer.GetColor(ColorRole::input_socket);
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

		break;
	}
	case model::BlockSocketModel::SocketType::output:
	{
		const SDL_Color color = renderer.GetColor(ColorRole::output_socket);
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
		break;
	}
	default:
	{
		assert(false);
		break;
	}
	}
	SDL_FPoint socket_pos = transformer.SpaceToScreenPoint(center);
	SDL_FRect draw_area = { socket_pos.x - socket_length.x / 2, socket_pos.y - socket_length.y / 2,
	socket_length.x, socket_length.y };
	SDL_RenderFillRect(renderer, &draw_area);
}

