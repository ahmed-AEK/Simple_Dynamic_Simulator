#pragma once

#include "NodeModels/Utils.hpp"
#include "NodeModels/IdTypes.hpp"
#include <vector>
#include <optional>
#include <span>
#include <cassert>
#include <memory>
#include <string>

namespace node::model
{

class BlockModel;

class BlockSocketModel
{
public:
	friend class BlockModel;

	enum class SocketType
	{
		input = 0,
		output= 1,
	};

	explicit BlockSocketModel(
		SocketType type, SocketId id, const Point& position = {},
		std::optional<NetNodeUniqueId> connectedNetNode = {}
	)
		: m_Id{ id }, m_position{ position }, m_type{ type },
		m_connectedNetNode{ connectedNetNode } {}

	const Point& GetPosition() const noexcept { return m_position; }
	void SetPosition(const Point& p) { m_position = p; }
	const SocketId& GetId() const noexcept { return m_Id; }
	void SetId(SocketId id) { m_Id = id; };

	const SocketType& GetType() const noexcept { return m_type; }
	const std::optional<NetNodeUniqueId>& GetConnectedNetNode() const noexcept { return m_connectedNetNode; }
	void SetConnectedNetNode(std::optional<NetNodeUniqueId> node_id) { m_connectedNetNode = node_id; }
	// SetConnectedNode in in Node to emit signals

private:
	SocketId m_Id;
	Point m_position;
	SocketType m_type;
	std::optional<NetNodeUniqueId> m_connectedNetNode;
};


class BlockModel
{
public:
	explicit BlockModel(const BlockId& id, const Rect& bounds = {})
		:m_bounds{ bounds }, m_Id{ id } {}

	void SetPosition(const Point& origin) { 
		m_bounds.x = origin.x;
		m_bounds.y = origin.y;
	}
	const Point GetPosition() const noexcept { return { m_bounds.x , m_bounds.y }; }

	void SetBounds(const Rect& bounds) { 
		m_bounds = bounds;
	}
	const Rect& GetBounds() const noexcept { return m_bounds; }

	void AddSocket(BlockSocketModel socket) 
	{
		m_sockets.push_back(std::move(socket));
	}

	std::optional<std::reference_wrapper<BlockSocketModel>>
		GetSocketById(SocketId id);


	auto GetSockets() const { 
		return std::span{ m_sockets };
	}
	auto GetSockets() {
		return std::span{ m_sockets };
	}

	const BlockId& GetId() const noexcept { return m_Id; }
	void SetId(const BlockId& id);
	void ReserveSockets(size_t size) { 
		m_sockets.reserve(size);
	}

private:
	Rect m_bounds;
	std::vector<BlockSocketModel> m_sockets;
	std::string block_styler;
	std::string block_class;
	BlockId m_Id;
};

using BlockModelRef = std::reference_wrapper<BlockModel>;

}