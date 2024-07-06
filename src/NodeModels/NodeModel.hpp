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

class NodeModel;

class NodeSocketModel
{
public:
	friend class NodeModel;

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

	const SocketId& GetId() const noexcept { return m_Id; }

	const SocketType& GetType() const noexcept { return m_type; }
	const std::optional<id_int> GetConnectedNetNode() const noexcept { return m_connectedNetNode; }
	// SetConnectedNode in in Node to emit signals

private:
	SocketId m_Id;
	Point m_position;
	SocketType m_type;
	std::optional<id_int> m_connectedNetNode;
};

using NodeSocketId = NodeSocketModel::SocketId;


enum class NodeEvent
{
	PositionChanged,
	BoundsChanged,
	SocketsChanged,
	SocketsRepositioned,
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
	}
	const Point& GetPosition() const noexcept { return m_bounds.origin; }

	void SetBounds(const Rect& bounds) { 
		m_bounds = bounds;
	}
	const Rect& GetBounds() const noexcept { return m_bounds; }

	void AddSocket(NodeSocketModel socket) 
	{
		assert(socket.GetId().m_nodeId == GetId());
		switch (socket.GetType())
		{
		case NodeSocketModel::SocketType::input:
		{
			m_input_sockets.push_back(std::move(socket));
			break;
		}
		case NodeSocketModel::SocketType::output:
		{
			m_output_sockets.push_back(std::move(socket));
			break;
		}
		case NodeSocketModel::SocketType::inout:
		{
			m_inout_sockets.push_back(std::move(socket));
			break;
		}
		}
	}

	std::optional<std::reference_wrapper<NodeSocketModel>>
		GetSocketById(id_int id, const NodeSocketModel::SocketType type);


	auto GetSockets(const NodeSocketModel::SocketType type) const { 
		switch (type)
		{
		case NodeSocketModel::SocketType::input:
			return std::span{ m_input_sockets };
		case NodeSocketModel::SocketType::output:
			return std::span{ m_output_sockets };
		case NodeSocketModel::SocketType::inout:
			return std::span{ m_inout_sockets };
		}
		return std::span<const NodeSocketModel>{};
	}
	auto GetSockets(const NodeSocketModel::SocketType type) {
		switch (type)
		{
		case NodeSocketModel::SocketType::input:
			return std::span{ m_input_sockets };
		case NodeSocketModel::SocketType::output:
			return std::span{ m_output_sockets };
		case NodeSocketModel::SocketType::inout:
			return std::span{ m_inout_sockets };
		}
		return std::span<NodeSocketModel>{};
	}

	const id_int& GetId() const noexcept { return m_Id; }

	void ReserveSockets(size_t size, const NodeSocketModel::SocketType type) { 
		switch (type)
		{
		case NodeSocketModel::SocketType::input:
			m_input_sockets.reserve(size);
			break;
		case NodeSocketModel::SocketType::output:
			m_output_sockets.reserve(size);
			break;
		case NodeSocketModel::SocketType::inout:
			m_inout_sockets.reserve(size);
			break;
		}
	}

private:
	Rect m_bounds;
	std::vector<NodeSocketModel> m_input_sockets;
	std::vector<NodeSocketModel> m_output_sockets;
	std::vector<NodeSocketModel> m_inout_sockets;
	id_int m_Id;
};

using NodeModelPtr = std::shared_ptr<NodeModel>;

}