#include "BlockStyler.hpp"

#include <algorithm>

void node::BlockStyler::PositionSockets(std::span<model::BlockSocketModel> sockets, const model::Rect& bounds) const
{
	auto in_sockets_count = std::count_if(sockets.begin(), sockets.end(),
		[](const model::BlockSocketModel& sock)
		{return sock.GetType() == model::BlockSocketModel::SocketType::input; });
	int in_spacing = static_cast<int>(bounds.h / (in_sockets_count + 1));
	auto out_sockets_count = sockets.size() - in_sockets_count;
	int out_spacing = static_cast<int>(bounds.h /
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
			sock.SetPosition({ bounds.w - 2 - SocketLength / 2,
				out_spacing * out_counter });
			out_counter++;
			break;
		}
		}
	}
}

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

void node::BlockStyler::DrawBlockDetails(SDL_Renderer* renderer, const model::Rect& bounds, const SpaceScreenTransformer& transformer, bool selected)
{
	UNUSED_PARAM(renderer);
	UNUSED_PARAM(bounds);
	UNUSED_PARAM(transformer);
	UNUSED_PARAM(selected);
}

void node::BlockStyler::UpdateProperties(const model::BlockModel& model)
{
	UNUSED_PARAM(model);
}

node::BlockStyler::~BlockStyler()
{

}
