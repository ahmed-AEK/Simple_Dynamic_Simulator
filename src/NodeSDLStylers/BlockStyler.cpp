#include "BlockStyler.hpp"

void node::BlockStyler::PositionNodes(model::BlockModel& block)
{
	{
		auto&& in_sockets = block.GetSockets(
			node::model::BlockSocketModel::SocketType::input);
		int in_spacing = static_cast<int>(block.GetBounds().h / (in_sockets.size() + 1));
		int in_counter = 1;
		for (auto&& sock : in_sockets)
		{
			sock.SetPosition({ 2,
				in_spacing * in_counter - SocketLength / 2 });
			in_counter++;
		}
	}
	{
		auto&& output_sockets = block.GetSockets(
			node::model::BlockSocketModel::SocketType::output);
		int out_spacing = static_cast<int>(block.GetBounds().h /
			(output_sockets.size() + 1));
		int out_counter = 1;
		for (auto&& sock : output_sockets)
		{
			sock.SetPosition({ block.GetBounds().w - 2 - SocketLength,
				out_spacing * out_counter - SocketLength / 2 });
			out_counter++;
		}
	}
}

void node::BlockStyler::DrawBlock(SDL_Renderer* renderer, const model::BlockModel& block, const SpaceScreenTransformer& transformer, bool selected)
{
	SDL_Rect screenRect = transformer.SpaceToScreenRect(block.GetBounds());
	if (!selected)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
	}
	SDL_RenderFillRect(renderer, &screenRect);
	SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
	SDL_Rect inner_rect{ screenRect.x + 2, screenRect.y + 2, screenRect.w - 4, screenRect.h - 4 };
	SDL_RenderFillRect(renderer, &inner_rect);
	SDL_Point socket_lengths = transformer.SpaceToScreenVector({ SocketLength, SocketLength });
	for (auto&& sock :
		block.GetSockets(node::model::BlockSocketModel::SocketType::input))
	{
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_Point socket_pos = transformer.SpaceToScreenVector(sock.GetPosition());
		SDL_Rect draw_area = { screenRect.x + socket_pos.x,
			screenRect.y + socket_pos.y,
		socket_lengths.x, socket_lengths.y};
		SDL_RenderFillRect(renderer, &draw_area);
	}
	for (auto&& sock :
		block.GetSockets(node::model::BlockSocketModel::SocketType::output))
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		SDL_Point socket_pos = transformer.SpaceToScreenVector(sock.GetPosition());
		SDL_Rect draw_area = { screenRect.x + socket_pos.x,
			screenRect.y + socket_pos.y,
		socket_lengths.x, socket_lengths.y };
		SDL_RenderFillRect(renderer, &draw_area);
	}
}