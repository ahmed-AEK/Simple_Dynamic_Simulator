#pragma once

#include "NodeModels/Utils.hpp"
#include <vector>
#include <optional>
#include <functional>
#include <span>
#include <cassert>

namespace node::model
{

enum class NodeSocketType
{
	input,
	output,
	inout,
};

struct NodeSocketId
{
	id_int m_Id = 0;
	id_int m_nodeId = 0;
};

class NodeSocketModel
{
public:
	explicit NodeSocketModel(
		NodeSocketType type, NodeSocketId id, const Point& position = {}
	)
		: m_position{ position }, m_Id{ id }, m_type{ type } {}

	const Point& GetPosition() const noexcept { return m_position; }
	void SetPosition(const Point& origin) { m_position = origin; }

	const NodeSocketId& GetId() const noexcept { return m_Id; }

	const NodeSocketType& GetType() const noexcept { return m_type; }
private:
	Point m_position;
	NodeSocketId m_Id;
	NodeSocketType m_type;
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

	std::span<NodeSocketModel>
		GetSockets() { return m_sockets; }

	const id_int GetId() const noexcept { return m_Id; }

	void ReserveSockets(size_t size) { m_sockets.reserve(size); }
private:
	Rect m_bounds;
	std::vector<NodeSocketModel> m_sockets;
	id_int m_Id;
};


}