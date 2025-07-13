#pragma once

#include <cstdint>
#include <vector>
#include <cassert>
#include <array>

#include "boost/container_hash/hash.hpp"

namespace node
{
	struct SubSceneId
	{
		SubSceneId() = default;
		explicit SubSceneId(int32_t value) : value{ value } {}
		bool operator==(const SubSceneId&) const = default;
		int32_t value{};
	};
}

namespace node::model
{

using id_int = int32_t;

enum class ConnectedSegmentSide : char
{
	north = 0,
	east = 1,
	south = 2,
	west = 3,
};

enum class NetSegmentOrientation : char
{
	horizontal = 0,
	vertical = 1,
};

inline ConnectedSegmentSide GetOppositeSegmentSide(ConnectedSegmentSide side)
{
	switch (side)
	{
		using enum ConnectedSegmentSide;
	case north:
		return south;
	case east:
		return west;
	case south:
		return north;
	case west:
		return east;
	}
	assert(false); // shouldn't reach here
	return {};
}

struct BlockId
{
	explicit BlockId(id_int value) : value{ value } {}
	bool operator==(const BlockId&) const = default;
	id_int value{};
};

struct SocketId
{
	explicit SocketId(id_int value) : value{ value } {}
	bool operator==(const SocketId&) const = default;
	id_int value{};
};

struct NetNodeId
{
	explicit NetNodeId(id_int value) : value{ value } {}
	bool operator<(const NetNodeId& other) const { return value < other.value; }
	bool operator==(const NetNodeId&) const = default;
	id_int value{};
};

struct NetSegmentId
{
	explicit NetSegmentId(id_int value) : value{ value } {}
	bool operator==(const NetSegmentId&) const = default;
	id_int value{};
};

struct NetId
{
	explicit NetId(id_int value) : value{ value } {}
	bool operator==(const NetId&) const = default;
	id_int value{};
};

struct SocketUniqueId
{
	bool operator==(const SocketUniqueId&) const = default;
	SocketId socket_id;
	BlockId   block_id;
};

class NetCategory
{
public:
	constexpr NetCategory(): category{} {}
	constexpr explicit NetCategory(std::string_view name) : category{}
	{
		assert(name.size() <= TEXT_SIZE);
		for (size_t i = 0; i < name.size() && i < TEXT_SIZE; i++)
		{
			category[i] = name[i];
		}
	}
	NetCategory(const NetCategory&) = default;
	NetCategory& operator=(const NetCategory&) = default;

	bool IsEmpty() const { return category[0] == '\0'; }
	friend bool operator==(const NetCategory&, const NetCategory&) = default;

	static bool Joinable(const NetCategory& first, const NetCategory& second)
	{
		if (first.IsEmpty() || second.IsEmpty())
		{
			return true;
		}
		if (first == second)
		{
			return true;
		}
		return false;
	}
	auto& buffer() { return category; }
	auto& buffer() const { return category; }
private:
	static constexpr size_t BUFFER_SIZE = 16;
	static constexpr size_t TEXT_SIZE = BUFFER_SIZE - 1;
	std::array<char, BUFFER_SIZE> category;
};

}

template <>
struct std::hash<node::SubSceneId>
{
	std::size_t operator()(const node::SubSceneId& k) const
	{
		return k.value;
	}
};

template <>
struct std::hash<node::model::NetId >
{
	std::size_t operator()(const node::model::NetId& k) const
	{
		return k.value;
	}
};

template <>
struct std::hash<node::model::NetCategory>
{
	std::size_t operator()(const node::model::NetCategory& k) const
	{
		size_t hash = 0;
		boost::hash_combine(hash, k.buffer());
		return hash;
	}
};

template <>
struct std::hash<node::model::BlockId>
{
	std::size_t operator()(const node::model::BlockId& k) const
	{
		return k.value;
	}
};

template <>
struct std::hash<node::model::NetSegmentId>
{
	std::size_t operator()(const node::model::NetSegmentId& k) const
	{
		return k.value;
	}
};

template <>
struct std::hash<node::model::SocketId>
{
	std::size_t operator()(const node::model::SocketId& k) const
	{
		return k.value;
	}
};

template <>
struct std::hash<node::model::NetNodeId>
{
	std::size_t operator()(const node::model::NetNodeId& k) const
	{
		return k.value;
	}
};

template <>
struct std::hash<node::model::SocketUniqueId>
{
	std::size_t operator()(const node::model::SocketUniqueId& k) const
	{
		size_t hash = 0;
		boost::hash_combine(hash, k.block_id.value);
		boost::hash_combine(hash, k.socket_id.value);
		return hash;
	}
};

