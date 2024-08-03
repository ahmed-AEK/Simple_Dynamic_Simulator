#pragma once

#include <cstdint>

namespace node::model
{

using id_int = int32_t;

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

struct NetSegmentUniqueId
{
	bool operator==(const NetSegmentUniqueId&) const = default;
	NetSegmentId segment_id;
	NetId        net_id;
};

struct NetNodeUniqueId
{
	bool operator==(const NetNodeUniqueId&) const = default;
	NetNodeId node_id;
	NetId     net_id;
};

}
