#include "BlockStyler.hpp"

#include <algorithm>

void node::BlockStyler::PositionSockets(std::span<model::BlockSocketModel> sockets, const model::Rect& bounds, model::BlockOrientation orientation) const
{
	int in_sockets_count = 0;
	int out_sockets_count = 0;
	int inout_sockets_count = 0;
	for (const auto& socket : sockets)
	{
		switch (socket.GetType())
		{
			using enum model::BlockSocketModel::SocketType;
		case input:
		{
			in_sockets_count++;
			break;
		}
		case output:
		{
			out_sockets_count++;
			break;
		}
		case inout:
		{
			inout_sockets_count++;
			break;
		}
		default:
		{
			assert(false);
		}
		}
	}
	int in_spacing;
	int out_spacing;
	int inout_spacing;

	switch (orientation)
	{
		using enum model::BlockOrientation;
	case TopToBottom:
	case BottomToTop:
		in_spacing = static_cast<int>(bounds.w / (in_sockets_count + 1));
		out_spacing = static_cast<int>(bounds.w / (out_sockets_count + 1));
		inout_spacing = static_cast<int>(bounds.h / (inout_sockets_count / 2 + 1));
		break;
	default:
		in_spacing = static_cast<int>(bounds.h / (in_sockets_count + 1));
		out_spacing = static_cast<int>(bounds.h / (out_sockets_count + 1));
		inout_spacing = static_cast<int>(bounds.w / (inout_sockets_count / 2 + 1));
		break;
	}

	int in_counter = 1;
	int out_counter = 1;
	int inout_counter = 2;
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
				sock.SetConnectionSide(model::ConnectedSegmentSide::west);
				break;
			case RightToLeft:
				sock.SetPosition({ bounds.w - 2 - SocketLength / 2,	in_spacing * in_counter });
				sock.SetConnectionSide(model::ConnectedSegmentSide::east);
				break;
			case TopToBottom:
				sock.SetPosition({ in_spacing * in_counter, 2 + SocketLength / 2 });
				sock.SetConnectionSide(model::ConnectedSegmentSide::north);
				break;
			case BottomToTop:
				sock.SetPosition({ in_spacing * in_counter, bounds.h - 2 - SocketLength / 2 });
				sock.SetConnectionSide(model::ConnectedSegmentSide::south);
				break;
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
				sock.SetConnectionSide(model::ConnectedSegmentSide::west);
				break;
			case LeftToRight:
				sock.SetPosition({ bounds.w - 2 - SocketLength / 2,	out_spacing * out_counter });
				sock.SetConnectionSide(model::ConnectedSegmentSide::east);
				break;
			case BottomToTop:
				sock.SetPosition({ out_spacing * out_counter, 2 + SocketLength / 2 });
				sock.SetConnectionSide(model::ConnectedSegmentSide::north);
				break;
			case TopToBottom:
				sock.SetPosition({ out_spacing * out_counter, bounds.h - 2 - SocketLength / 2 });
				sock.SetConnectionSide(model::ConnectedSegmentSide::south);
				break;
			}
			out_counter++;
			break;
		}
		case model::BlockSocketModel::SocketType::inout:
		{
			if (inout_counter % 2)
			{
				switch (orientation)
				{
					using enum model::BlockOrientation;
				case RightToLeft:
					sock.SetPosition({ inout_spacing * (inout_counter / 2), 2 + SocketLength / 2 });
					sock.SetConnectionSide(model::ConnectedSegmentSide::north);
					break;
				case LeftToRight:
					sock.SetPosition({inout_spacing * (inout_counter / 2), bounds.h - 2 - SocketLength / 2 });
					sock.SetConnectionSide(model::ConnectedSegmentSide::south);
					break;
				case BottomToTop:
					sock.SetPosition({ 2 + SocketLength / 2, inout_spacing * (inout_counter / 2) });
					sock.SetConnectionSide(model::ConnectedSegmentSide::west);
					break;
				case TopToBottom:
					sock.SetPosition({ bounds.w - 2 - SocketLength / 2, inout_spacing * (inout_counter / 2) });
					sock.SetConnectionSide(model::ConnectedSegmentSide::east);
					break;
				}
			}
			else
			{
				switch (orientation)
				{
					using enum model::BlockOrientation;
				case LeftToRight:
					sock.SetPosition({ inout_spacing * (inout_counter / 2), 2 + SocketLength / 2 });
					sock.SetConnectionSide(model::ConnectedSegmentSide::north);
					break;
				case RightToLeft:
					sock.SetPosition({ inout_spacing * (inout_counter / 2), bounds.h - 2 - SocketLength / 2 });
					sock.SetConnectionSide(model::ConnectedSegmentSide::south);
					break;
				case TopToBottom:
					sock.SetPosition({ 2 + SocketLength / 2, inout_spacing * (inout_counter / 2) });
					sock.SetConnectionSide(model::ConnectedSegmentSide::west);
					break;
				case BottomToTop:
					sock.SetPosition({ bounds.w - 2 - SocketLength / 2, inout_spacing * (inout_counter / 2) });
					sock.SetConnectionSide(model::ConnectedSegmentSide::east);
					break;
				}
			}
			inout_counter++;
			break;
		}
		}
	}
}

void node::BlockStyler::DrawBlock(SDL::Renderer& renderer, const model::BlockModel& model, const SpaceScreenTransformer& transformer, bool selected)
{
	auto&& bounds = model.GetBounds();
	DrawBlockOutline(renderer, bounds, transformer, model.GetOrienation(), selected);

	for (const auto& socket : model.GetSockets())
	{
		DrawBlockSocket(renderer, socket.GetPosition(), transformer, socket.GetType());
	}

	DrawBlockDetails(renderer, bounds, transformer, model.GetOrienation(), selected);
}

void node::BlockStyler::DrawBlockDetails(SDL::Renderer& renderer, const model::Rect& bounds, const SpaceScreenTransformer& transformer,
	model::BlockOrientation orientation, bool selected)
{
	UNUSED_PARAM(orientation);
	UNUSED_PARAM(renderer);
	UNUSED_PARAM(bounds);
	UNUSED_PARAM(transformer);
	UNUSED_PARAM(selected);
}

void node::BlockStyler::UpdateProperties(const model::BlockDataCRef& model)
{
	UNUSED_PARAM(model);
}

node::BlockStyler::~BlockStyler()
{

}
