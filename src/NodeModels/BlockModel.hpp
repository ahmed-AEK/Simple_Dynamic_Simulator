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
#include <string>

namespace node::model
{

class BlockModel;

class BlockSocketModel
{
public:
	friend class BlockModel;

	struct SocketId
	{
		id_int m_Id = 0;
		id_int m_nodeId = 0;
	};

	enum class SocketType
	{
		input = 0,
		output= 1,
	};

	explicit BlockSocketModel(
		SocketType type, SocketId id, const Point& position = {},
		std::optional<id_int> connectedNetNode = {}
	)
		: m_Id{ id }, m_position{ position }, m_type{ type },
		m_connectedNetNode{ connectedNetNode } {}

	const Point& GetPosition() const noexcept { return m_position; }
	void SetPosition(const Point& p) { m_position = p; }
	const SocketId& GetId() const noexcept { return m_Id; }
	void SetId(SocketId id) { m_Id = id; };

	const SocketType& GetType() const noexcept { return m_type; }
	const std::optional<id_int> GetConnectedNetNode() const noexcept { return m_connectedNetNode; }
	// SetConnectedNode in in Node to emit signals

private:
	SocketId m_Id;
	Point m_position;
	SocketType m_type;
	std::optional<id_int> m_connectedNetNode;
};

using BlockSocketId = BlockSocketModel::SocketId;

class BlockModel
{
public:
	explicit BlockModel(id_int id, const Rect& bounds = {})
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
		assert(socket.GetId().m_nodeId == GetId());
		switch (socket.GetType())
		{
		case BlockSocketModel::SocketType::input:
		{
			m_input_sockets.push_back(std::move(socket));
			break;
		}
		case BlockSocketModel::SocketType::output:
		{
			m_output_sockets.push_back(std::move(socket));
			break;
		}
		}
	}

	std::optional<std::reference_wrapper<BlockSocketModel>>
		GetSocketById(id_int id, const BlockSocketModel::SocketType type);


	auto GetSockets(const BlockSocketModel::SocketType type) const { 
		switch (type)
		{
		case BlockSocketModel::SocketType::input:
			return std::span{ m_input_sockets };
		case BlockSocketModel::SocketType::output:
			return std::span{ m_output_sockets };
		}
		return std::span<const BlockSocketModel>{};
	}
	auto GetSockets(const BlockSocketModel::SocketType type) {
		switch (type)
		{
		case BlockSocketModel::SocketType::input:
			return std::span{ m_input_sockets };
		case BlockSocketModel::SocketType::output:
			return std::span{ m_output_sockets };
		}
		return std::span<BlockSocketModel>{};
	}

	const id_int& GetId() const noexcept { return m_Id; }
	void SetId(const id_int& id);
	void ReserveSockets(size_t size, const BlockSocketModel::SocketType type) { 
		switch (type)
		{
		case BlockSocketModel::SocketType::input:
			m_input_sockets.reserve(size);
			break;
		case BlockSocketModel::SocketType::output:
			m_output_sockets.reserve(size);
			break;
		}
	}

private:
	Rect m_bounds;
	std::vector<BlockSocketModel> m_input_sockets;
	std::vector<BlockSocketModel> m_output_sockets;
	std::string block_styler;
	std::string block_class;
	id_int m_Id;
};

using BlockModelPtr = std::shared_ptr<BlockModel>;

}