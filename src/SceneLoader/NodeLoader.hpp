#pragma once

#include "NodeModels/NodeScene.hpp"

namespace node::loader
{


class NodeLoader
{
public:
	virtual bool AddBlock(const node::model::BlockModel& node) = 0;
	virtual bool DeleteBlockAndSockets(const node::model::BlockId& node_id) = 0;
	virtual bool UpdateBlockPosition(const node::model::BlockId& node_id,
		const node::model::Point& position) = 0;
	virtual	std::optional<node::model::BlockModel>
		GetBlock(const node::model::BlockId& block_id) = 0;
	virtual bool UpdateBlockBounds(const node::model::BlockId& node_id,
		const node::model::Rect& bounds) = 0;

	virtual bool AddSocket(const node::model::BlockSocketModel& socket, const model::BlockId& block_id) = 0;
	virtual bool DeleteSocket(const node::model::SocketUniqueId& socket_id) = 0;
	virtual bool UpdateSocketPosition(const node::model::SocketUniqueId& socket_id,
		const node::model::Point& position) = 0;

	virtual node::model::BlockId GetNextBlockId() = 0;
	virtual std::vector<node::model::BlockModel> GetBlocks() = 0;

	virtual ~NodeLoader() = default;
	NodeLoader() = default;
	NodeLoader(NodeLoader&&) = default;
	NodeLoader(const NodeLoader&) = default;
	NodeLoader& operator=(NodeLoader&&) = default;
	NodeLoader& operator=(const NodeLoader&) = default;
};

}