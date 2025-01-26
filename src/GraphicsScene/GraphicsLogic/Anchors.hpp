#pragma once

#include "toolgui/Handle.hpp"
#include "NodeModels/Utils.hpp"

#include <span>
#include <variant>
#include <array>

namespace node
{
class BlockSocketObject;
class GraphicsObject;
class NetNode;
class GraphicsObject;
class NetSegment;

namespace logic
{


struct SocketAnchor
{
	HandlePtrS<BlockSocketObject, GraphicsObject> socket;
	model::Point position;
};

struct NodeAnchor
{
	HandlePtrS<NetNode, GraphicsObject> node;
	model::Point position;
	std::array<bool, 4> allowed_sides;
};

using anchor_t = typename std::variant<std::monostate, SocketAnchor, NodeAnchor>;


struct AnchorAlive
{
	bool operator()(const std::monostate&);
	bool operator()(const node::logic::SocketAnchor& socket);
	bool operator()(const node::logic::NodeAnchor& node);
};


struct AnchorStart
{
	node::model::Point operator()(const std::monostate&);
	node::model::Point operator()(const node::logic::SocketAnchor& socket);
	node::model::Point operator()(const node::logic::NodeAnchor& node);
};

struct AnchorGetConnectionSide
{
	std::array<bool, 4> operator()(const std::monostate&);
	std::array<bool, 4> operator()(const node::logic::SocketAnchor& socket);
	std::array<bool, 4> operator()(const node::logic::NodeAnchor& node);
};

anchor_t CreateStartAnchor(std::span<HandlePtrS<NetNode,GraphicsObject>> nodes, std::span<HandlePtrS<NetSegment,GraphicsObject>> segments);
}
}
