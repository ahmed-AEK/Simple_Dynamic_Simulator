#pragma once

#include "NodeModels/Utils.hpp"
#include <vector>
#include <optional>
#include <functional>
#include <span>
#include <cassert>
#include "NodeModels/Observer.hpp"
#include <variant>
#include <memory>

namespace node::model
{

class NodeSocketModel
{
public:

	struct SocketId
	{
		id_int m_Id = 0;
		id_int m_nodeId = 0;
	};

	enum class SocketType
	{
		input = 0,
		output= 1,
		inout = 2,
	};

	explicit NodeSocketModel(
		SocketType type, SocketId id, const Point& position = {},
		std::optional<id_int> connectedNetNode = {}
	)
		: m_Id{ id }, m_position{ position }, m_type{ type },
		m_connectedNetNode{ connectedNetNode } {}

	const Point& GetPosition() const noexcept { return m_position; }
	void SetPosition(const Point& origin) { m_position = origin; }

	const SocketId& GetId() const noexcept { return m_Id; }

	const SocketType& GetType() const noexcept { return m_type; }
	const std::optional<id_int> GetConnectedNetNode() const noexcept { return m_connectedNetNode; }
	void SetConnectedNetNode(id_int node_id) { m_connectedNetNode = node_id; }

private:
	SocketId m_Id;
	Point m_position;
	SocketType m_type;
	std::optional<id_int> m_connectedNetNode;
};

using NodeSocketId = NodeSocketModel::SocketId;

class NodeModel;

enum class NodeEvent
{
	PositionChanged,
	BoundsChanged,
	SocketsChanged,
	SocketConnected,
};

struct NodeEventArg
{
	NodeModel& object;
	const NodeEvent event;
};

class NodeModel : public Publisher<NodeEventArg>
{
public:
	explicit NodeModel(id_int id, const Rect& bounds = {})
		:m_bounds{ bounds }, m_Id{ id } {}

	void SetPosition(const Point& origin) { 
		m_bounds.origin = origin; 
		NodeEventArg event{ *this, NodeEvent::PositionChanged };
		Notify(event);
	}
	const Point& GetPosition() const noexcept { return m_bounds.origin; }

	void SetBounds(const Rect& bounds) { 
		m_bounds = bounds;
		NodeEventArg event{ *this, NodeEvent::BoundsChanged };
		Notify(event);
	}
	const Rect& GetBounds() const noexcept { return m_bounds; }

	void AddSocket(NodeSocketModel socket) 
	{
		assert(socket.GetId().m_nodeId == GetId());
		m_sockets.push_back(std::move(socket));
		NodeEventArg event{ *this, NodeEvent::SocketsChanged };
		Notify(event);
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

using NodeModelPtr = std::shared_ptr<NodeModel>;

}