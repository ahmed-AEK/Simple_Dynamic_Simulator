#pragma once

#include <cstdint>
#include <vector>
#include <cassert>

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

