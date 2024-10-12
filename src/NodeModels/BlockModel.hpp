#pragma once

#include "NodeModels/Utils.hpp"
#include "NodeModels/IdTypes.hpp"
#include <vector>
#include <optional>
#include <span>
#include <cassert>
#include <memory>
#include <string>
#include <variant>
#include <unordered_map>

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
		std::optional<NetNodeId> connectedNetNode = {}
	)
		: m_Id{ id }, m_position{ position }, m_type{ type },
		m_connectedNetNode{ connectedNetNode } {}

	const Point& GetPosition() const noexcept { return m_position; }
	void SetPosition(const Point& p) { m_position = p; }
	const SocketId& GetId() const noexcept { return m_Id; }
	void SetId(SocketId id) { m_Id = id; };

	const SocketType& GetType() const noexcept { return m_type; }
	const std::optional<NetNodeId>& GetConnectedNetNode() const noexcept { return m_connectedNetNode; }
	void SetConnectedNetNode(std::optional<NetNodeId> node_id) { m_connectedNetNode = node_id; }
	// SetConnectedNode in in Node to emit signals

private:
	SocketId m_Id;
	Point m_position;
	SocketType m_type;
	std::optional<NetNodeId> m_connectedNetNode;
};

enum class BlockPropertyType
{
	String,
	FloatNumber, // float
	Integer, // maybe negative
	UnsignedInteger, // unsigned
	Boolean,
};

struct BlockProperty
{
	using property_t = std::variant<std::string, double, int64_t, uint64_t, bool>;
	BlockProperty(std::string name, BlockPropertyType type, property_t prop)
		:name{ name }, type{ type }, prop{ prop } {}
	std::string name;
	BlockPropertyType type;
	property_t prop;

	std::string to_string() const;
	static std::optional<property_t> from_string(BlockPropertyType type, std::string_view str);
	static std::optional<property_t> from_string(BlockPropertyType type, std::string&& str);
};

struct BlockStyleProperties
{
	std::unordered_map<std::string, std::string> properties;
};

enum class BlockOrientation
{
	LeftToRight,
	TopToBottom,
	RightToLeft,
	BottomToTop,
};

class BlockModel
{
public:
	explicit BlockModel(const BlockId& id, const Rect& bounds = {}, BlockOrientation orientaion = BlockOrientation::LeftToRight)
		:m_bounds{ bounds }, m_Id{ id }, m_orientation{orientaion} {}

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

	std::optional<std::reference_wrapper<BlockSocketModel>>
		GetSocketById(SocketId id);

	std::optional<std::reference_wrapper<const BlockSocketModel>>
		GetSocketById(SocketId id) const;


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

	const std::string& GetClass() const { return m_block_class; }
	void SetClass(std::string block_class) { m_block_class = block_class; }

	const std::string& GetStyler() const { return m_block_styler; }
	void SetStyler(std::string block_styler) { m_block_styler = block_styler; }

	const std::vector<BlockProperty>& GetProperties() const { return m_properties; }
	std::vector<BlockProperty>& GetProperties() { return m_properties; }

	void SetStylerProperties(BlockStyleProperties properties) { m_stylerProperties = std::move(properties); }
	const BlockStyleProperties& GetStylerProperties() const { return m_stylerProperties; }

	void SetOrientation(BlockOrientation orientation) { m_orientation = orientation; }
	BlockOrientation GetOrienation() const { return m_orientation; }

private:
	Rect m_bounds;
	std::vector<BlockSocketModel> m_sockets;
	std::vector<BlockProperty> m_properties;
	std::string m_block_styler;
	std::string m_block_class;
	BlockStyleProperties m_stylerProperties;
	BlockId m_Id;
	BlockOrientation m_orientation;
};

using BlockModelRef = std::reference_wrapper<BlockModel>;
using BlockModelConstRef = std::reference_wrapper<const BlockModel>;


}