#pragma once

#include <cstdint>
#include <vector>

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

enum class ConnectedSegmentSide
{
	north = 0,
	south = 1,
	west = 2,
	east = 3,
};

enum class NetSegmentOrientation
{
	horizontal = 0,
	vertical = 1,
};

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
		size_t hash = 23;
		hash = hash * 31 + k.block_id.value;
		hash = hash * 31 + k.socket_id.value;
		return hash;
	}
};

