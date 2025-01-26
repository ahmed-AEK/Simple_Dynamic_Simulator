#include "GraphicsLogic/Anchors.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/GraphicsObject.hpp"
#include "GraphicsScene/NetObject.hpp"

namespace node::logic
{

bool AnchorAlive::operator()(const std::monostate&)
{
	return {};
}
bool AnchorAlive::operator()(const node::logic::SocketAnchor& socket)
{
	return socket.socket.isAlive();
}
bool AnchorAlive::operator()(const node::logic::NodeAnchor& node)
{
	return node.node.isAlive();
}


node::model::Point AnchorStart::operator()(const std::monostate&)
{
	return {};
}

node::model::Point AnchorStart::operator()(const node::logic::SocketAnchor& socket)
{
	return socket.position;
}
node::model::Point AnchorStart::operator()(const node::logic::NodeAnchor& node)
{
	return node.position;
}


std::array<bool, 4> AnchorGetConnectionSide::operator()(const std::monostate&)
{
	return {};
}
std::array<bool, 4> AnchorGetConnectionSide::operator()(const node::logic::SocketAnchor& socket)
{
	std::array<bool, 4> sides{};
	sides[static_cast<int>(socket.socket->GetConnectionSide())] = true;
	return sides;
}
std::array<bool, 4> AnchorGetConnectionSide::operator()(const node::logic::NodeAnchor& node)
{
	return node.allowed_sides;
}

anchor_t CreateStartAnchor(std::span<HandlePtrS<NetNode, GraphicsObject>> nodes, 
	std::span<HandlePtrS<NetSegment, GraphicsObject>> segments)
{
	auto* node = nodes[0].GetObjectPtr();
	if (auto* socket = nodes[0]->GetConnectedSocket())
	{
		return SocketAnchor{ HandlePtrS<BlockSocketObject,GraphicsObject>{*socket }, node->getCenter() };
	}
	else
	{
		auto* orig_segment = segments[0].GetObjectPtr();
		std::array<bool, 4> sides{};
		for (int i = 0; i < 4; i++)
		{
			if (auto* segment = node->getSegment(static_cast<model::ConnectedSegmentSide>(i)); !segment || segment == orig_segment)
			{
				sides[i] = true;
			}
		}
		return NodeAnchor{ HandlePtrS<NetNode, GraphicsObject> {*node}, node->getCenter(), sides };
	}
}

}