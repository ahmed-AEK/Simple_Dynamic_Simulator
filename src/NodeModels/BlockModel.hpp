#pragma once

#include "NodeModels/Utils.hpp"
#include "NodeModels/IdTypes.hpp"
#include <vector>
#include <optional>
#include <span>
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

namespace node::model
{

class BlockModel;

enum class SocketType : char
{
	input = 0,
	output = 1,
	inout = 2,
};

class BlockSocketModel
{
public:

	using SocketType = model::SocketType;

	explicit BlockSocketModel(
		SocketType type, SocketId id, const Point& position = {}, ConnectedSegmentSide side = {},
		std::optional<NetNodeId> connectedNetNode = {}, const NetCategory& category = {}
	)
		: m_Id{ id }, m_position{ position }, m_type{ type }, m_side{side},
		m_connectedNetNode{ connectedNetNode }, m_category{ category } {}
	const Point& GetPosition() const noexcept { return m_position; }
	void SetPosition(const Point& p) { m_position = p; }
	const SocketId& GetId() const noexcept { return m_Id; }
	void SetId(SocketId id) { m_Id = id; };

	void SetCategory(const NetCategory& category) { m_category = category; }
	const NetCategory& GetCategory() const { return m_category; }

	const SocketType& GetType() const noexcept { return m_type; }
	ConnectedSegmentSide GetConnectionSide() const { return m_side; }
	void SetConnectionSide(ConnectedSegmentSide side) { m_side = side; }
	const std::optional<NetNodeId>& GetConnectedNetNode() const noexcept { return m_connectedNetNode; }
	void SetConnectedNetNode(std::optional<NetNodeId> node_id) { m_connectedNetNode = node_id; }
	// SetConnectedNode in in Node to emit signals

private:
	SocketId m_Id;
	Point m_position;
	SocketType m_type;
	ConnectedSegmentSide m_side;
	std::optional<NetNodeId> m_connectedNetNode;
	NetCategory m_category;
};



struct BlockStyleProperties
{
	std::unordered_map<std::string, std::string> properties;
};

enum class BlockOrientation : uint8_t
{
	LeftToRight,
	TopToBottom,
	RightToLeft,
	BottomToTop,
};

enum class BlockType: uint8_t
{
	Functional,
	SubSystem,
	Port,
};

class BlockModel
{
public:
	explicit BlockModel(const BlockId& id, BlockType block_type, const Rect& bounds = {}, BlockOrientation orientaion = BlockOrientation::LeftToRight)
		:m_bounds{ bounds }, m_Id{ id }, m_orientation{ orientaion }, m_type{block_type} {}

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

	void ClearSockets()
	{
		m_sockets.clear();
	}

	BlockSocketModel* GetSocketById(SocketId id);

	const BlockSocketModel* GetSocketById(SocketId id) const;


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

	BlockType GetType() const { return m_type; }
	void SetType(BlockType t) { m_type = t; }

	const std::string& GetStyler() const { return m_block_styler; }
	void SetStyler(std::string block_styler) { m_block_styler = block_styler; }

	void SetStylerProperties(BlockStyleProperties properties) { m_stylerProperties = std::move(properties); }
	const BlockStyleProperties& GetStylerProperties() const { return m_stylerProperties; }

	void SetOrientation(BlockOrientation orientation) { m_orientation = orientation; }
	BlockOrientation GetOrienation() const { return m_orientation; }

private:
	Rect m_bounds;
	std::vector<BlockSocketModel> m_sockets;
	std::string m_block_styler;
	BlockStyleProperties m_stylerProperties;
	BlockId m_Id;
	BlockOrientation m_orientation;
	BlockType m_type;
};

using BlockModelRef = std::reference_wrapper<BlockModel>;
using BlockModelConstRef = std::reference_wrapper<const BlockModel>;

}