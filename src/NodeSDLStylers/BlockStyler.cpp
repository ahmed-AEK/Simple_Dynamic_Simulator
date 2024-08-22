#include "BlockStyler.hpp"
#include <algorithm>
#include "SDL_Framework/Utility.hpp"

void node::BlockStyler::PositionNodes(model::BlockModel& block)
{
	{
		auto&& sockets = block.GetSockets();
		auto in_sockets_count = std::count_if(sockets.begin(), sockets.end(),
			[](const model::BlockSocketModel& sock)
			{return sock.GetType() == model::BlockSocketModel::SocketType::input; });
		int in_spacing = static_cast<int>(block.GetBounds().h / (in_sockets_count + 1));
		auto out_sockets_count = sockets.size() - in_sockets_count;
		int out_spacing = static_cast<int>(block.GetBounds().h /
			(out_sockets_count + 1));
		int in_counter = 1;
		int out_counter = 1;
		for (auto&& sock : sockets)
		{
			switch (sock.GetType())
			{
			case model::BlockSocketModel::SocketType::input:
			{
				sock.SetPosition({ 2 + SocketLength / 2,
					in_spacing * in_counter });
				in_counter++;
				break;
			}
			case model::BlockSocketModel::SocketType::output:
			{
				sock.SetPosition({ block.GetBounds().w - 2 - SocketLength / 2,
					out_spacing * out_counter });
				out_counter++;
				break;
			}
		}
	}
	}
}

void node::BlockStyler::DrawBlock(SDL_Renderer* renderer, const model::BlockModel& model, const SpaceScreenTransformer& transformer, bool selected)
{
	SDL_Rect screenRect = transformer.SpaceToScreenRect(model.GetBounds());
	SDL_Color outer_color = selected ? SDL_Color{ 255,165,0,255 } : SDL_Color{ 0,0,0,255 };
	SDL_Color inner_color{ 220,220,220,255 };
	
	ThickFilledRoundRect(renderer, screenRect, screenRect.w / 10, 2, outer_color, inner_color,
		m_outer_painter, m_inner_painter);

	SDL_Point socket_length = transformer.SpaceToScreenVector({ SocketLength, SocketLength });
	for (const auto& socket : model.GetSockets())
	{
		switch (socket.GetType())
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
		SDL_Point socket_pos = transformer.SpaceToScreenPoint(socket.GetPosition());
		SDL_Rect draw_area = { socket_pos.x - socket_length.x / 2, socket_pos.y - socket_length.y / 2,
		socket_length.x, socket_length.y };
		SDL_RenderFillRect(renderer, &draw_area);
	}
}

void node::BlockStyler::DrawBlockOutline(SDL_Renderer* renderer, const model::Rect& bounds, const SpaceScreenTransformer& transformer, bool selected)
{
	SDL_Rect screenRect = transformer.SpaceToScreenRect(bounds);
	SDL_Color outer_color = selected ? SDL_Color{ 255,165,0,255 } : SDL_Color{ 0,0,0,255 };
	SDL_Color inner_color{ 220,220,220,255 };

	ThickFilledRoundRect(renderer, screenRect, screenRect.w/10, 2, outer_color, inner_color,
		m_outer_painter, m_inner_painter);
}

void node::BlockStyler::DrawBlockSocket(SDL_Renderer* renderer, const model::Point& center, const SpaceScreenTransformer& transformer, 
	const model::BlockSocketModel::SocketType& type)
{
	SDL_Point socket_length = transformer.SpaceToScreenVector({ SocketLength, SocketLength });
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
	SDL_Point socket_pos = transformer.SpaceToScreenPoint(center);
	SDL_Rect draw_area = { socket_pos.x - socket_length.x / 2, socket_pos.y - socket_length.y / 2,
	socket_length.x, socket_length.y };
	SDL_RenderFillRect(renderer, &draw_area);
}
