#pragma once

#include "NodeModels/Utils.hpp"
#include <vector>
#include <optional>
#include <functional>
#include <span>
#include <cassert>

namespace node::model
{



struct NodeSocketId
{
	id_int m_Id = 0;
	id_int m_nodeId = 0;
};

class NodeSocketModel
{
public:

	enum class SocketType
	{
		input = 0,
		output= 1,
		inout = 2,
	};

	explicit NodeSocketModel(
		SocketType type, NodeSocketId id, const Point& position = {},
		std::optional<id_int> connectedNetNode = {}
	)
		: m_Id{ id }, m_position{ position }, m_type{ type },
		m_connectedNetNode{ connectedNetNode } {}

	const Point& GetPosition() const noexcept { return m_position; }
	void SetPosition(const Point& origin) { m_position = origin; }

	const NodeSocketId& GetId() const noexcept { return m_Id; }

	const SocketType& GetType() const noexcept { return m_type; }
	const std::optional<id_int> GetConnectedNetNode() const noexcept { return m_connectedNetNode; }
	void SetConnectedNetNode(id_int node_id) { m_connectedNetNode = node_id; }

private:
	NodeSocketId m_Id;
	Point m_position;
	SocketType m_type;
	std::optional<id_int> m_connectedNetNode;
};

class NodeModel
{
public:
	explicit NodeModel(id_int id, const Rect& bounds = {})
		:m_bounds{ bounds }, m_Id{ id } {}

	void SetPosition(const Point& origin) { m_bounds.origin = origin; }
	const Point& GetPosition() const noexcept { return m_bounds.origin; }

	void SetBounds(const Rect& bounds) { m_bounds = bounds; }
	const Rect& GetBounds() const noexcept { return m_bounds; }

	void AddSocket(NodeSocketModel socket) 
	{
		assert(socket.GetId().m_nodeId == GetId());
		m_sockets.push_back(std::move(socket));
	}
	void RemoveSocketById(id_int id);

	std::optional<std::reference_wrapper<NodeSocketModel>>
		GetSocketById(id_int id);

	auto GetSockets() const { return std::span{ m_sockets }; }
	auto GetSockets() { return std::span{ m_sockets }; }

	const id_int& GetId() const noexcept { return m_Id; }

	void ReserveSockets(size_t size) { m_sockets.reserve(size); }
private:
	Rect m_bounds;
	std::vector<NodeSocketModel> m_sockets;
	id_int m_Id;
};


}