#include "DefaultBlockStyler.hpp"
#include <algorithm>
#include "SDL_Framework/Utility.hpp"

void node::DefaultBlockStyler::DrawBlockOutline(SDL_Renderer* renderer, const model::Rect& bounds, const SpaceScreenTransformer& transformer, 
	model::BlockOrientation orientation, bool selected)
{
	UNUSED_PARAM(orientation);
	SDL_FRect screenRect = transformer.SpaceToScreenRect(bounds);
	SDL_Color outer_color = selected ? SDL_Color{ 255,165,0,255 } : SDL_Color{ 0,0,0,255 };
	SDL_Color inner_color{ 220,220,220,255 };

	ThickFilledRoundRect(renderer, screenRect, static_cast<int>(screenRect.w / 10), 2, outer_color, inner_color,
		m_outer_painter, m_inner_painter);
}

void node::DefaultBlockStyler::DrawBlockSocket(SDL_Renderer* renderer, const model::Point& center, const SpaceScreenTransformer& transformer,
	const model::BlockSocketModel::SocketType& type)
{
	SDL_FPoint socket_length = transformer.SpaceToScreenVector({ SocketLength, SocketLength });
	switch (type)
	{
	case model::BlockSocketModel::SocketType::input:
	{
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

		break;
	}
	case model::BlockSocketModel::SocketType::output:
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
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

