#include "BlockStyler.hpp"

#include <algorithm>

void node::BlockStyler::PositionSockets(std::span<model::BlockSocketModel> sockets, const model::Rect& bounds, model::BlockOrientation orientation) const
{
	auto in_sockets_count = std::count_if(sockets.begin(), sockets.end(),
		[](const model::BlockSocketModel& sock)
		{return sock.GetType() == model::BlockSocketModel::SocketType::input; });
	
	int in_spacing;
	
	auto out_sockets_count = sockets.size() - in_sockets_count;
	int out_spacing;
	switch (orientation)
	{
		using enum model::BlockOrientation;
	case TopToBottom:
	case BottomToTop:
		in_spacing = static_cast<int>(bounds.w / (in_sockets_count + 1));
		out_spacing = static_cast<int>(bounds.w /
			(out_sockets_count + 1));
		break;
	default:
		in_spacing = static_cast<int>(bounds.h / (in_sockets_count + 1));
		out_spacing = static_cast<int>(bounds.h /
			(out_sockets_count + 1));
		break;
	}

	int in_counter = 1;
	int out_counter = 1;
	for (auto&& sock : sockets)
	{
		switch (sock.GetType())
		{
		case model::BlockSocketModel::SocketType::input:
		{
			switch (orientation)
			{
				using enum model::BlockOrientation;
			case LeftToRight:
				sock.SetPosition({ 2 + SocketLength / 2, in_spacing * in_counter });
				break;
			case RightToLeft:
				sock.SetPosition({ bounds.w - 2 - SocketLength / 2,	in_spacing * in_counter });
				break;
			case TopToBottom:
				sock.SetPosition({ in_spacing * in_counter, 2 + SocketLength / 2 });
				break;
			case BottomToTop:
				sock.SetPosition({ in_spacing * in_counter, bounds.h - 2 - SocketLength / 2 });
			}

			in_counter++;
			break;
		}
		case model::BlockSocketModel::SocketType::output:
		{
			switch (orientation)
			{
				using enum model::BlockOrientation;
			case RightToLeft:
				sock.SetPosition({ 2 + SocketLength / 2, out_spacing * out_counter });
				break;
			case LeftToRight:
				sock.SetPosition({ bounds.w - 2 - SocketLength / 2,	out_spacing * out_counter });
				break;
			case BottomToTop:
				sock.SetPosition({ out_spacing * out_counter, 2 + SocketLength / 2 });
				break;
			case TopToBottom:
				sock.SetPosition({ out_spacing * out_counter, bounds.h - 2 - SocketLength / 2 });
			}
			out_counter++;
			break;
		}
		}
	}
}

void node::BlockStyler::DrawBlock(SDL_Renderer* renderer, const model::BlockModel& model, const SpaceScreenTransformer& transformer, bool selected)
{
	auto&& bounds = model.GetBounds();
	DrawBlockOutline(renderer, bounds, transformer, model.GetOrienation(), selected);

	for (const auto& socket : model.GetSockets())
	{
		DrawBlockSocket(renderer, socket.GetPosition(), transformer, socket.GetType());
	}

	DrawBlockDetails(renderer, bounds, transformer, model.GetOrienation(), selected);
}

void node::BlockStyler::DrawBlockDetails(SDL_Renderer* renderer, const model::Rect& bounds, const SpaceScreenTransformer& transformer, 
	model::BlockOrientation orientation, bool selected)
{
	UNUSED_PARAM(orientation);
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
