#pragma once

#include "NodeModels/NodeScene.hpp"

namespace node::loader
{


class BlockLoader
{
public:
	virtual bool AddBlock(const node::model::BlockModel& block) = 0;
	virtual bool DeleteBlockAndSockets(const node::model::BlockId& block_id) = 0;
	virtual bool UpdateBlockPosition(const node::model::BlockId& block_id,
		const node::model::Point& position) = 0;
	virtual	std::optional<node::model::BlockModel>
		GetBlock(const node::model::BlockId& block_id) = 0;
	virtual bool UpdateBlockBounds(const node::model::BlockId& block_id,
		const node::model::Rect& bounds) = 0;

	virtual bool AddStylerProperty(const node::model::BlockId& block_id, const std::string& name, const std::string& value, int32_t property_id) = 0;
	virtual bool AddProperty(const node::model::BlockId& block_id, const model::BlockProperty& property, int32_t property_id) = 0;
	virtual bool AddSocket(const node::model::BlockSocketModel& socket, const model::BlockId& block_id) = 0;
	virtual bool DeleteSocket(const node::model::SocketUniqueId& socket_id) = 0;
	virtual bool UpdateSocketPosition(const node::model::SocketUniqueId& socket_id,
		const node::model::Point& position) = 0;

	virtual node::model::BlockId GetNextBlockId() = 0;
	virtual std::vector<node::model::BlockModel> GetBlocks() = 0;

	virtual ~BlockLoader() = default;
	BlockLoader() = default;
	BlockLoader(BlockLoader&&) = default;
	BlockLoader(const BlockLoader&) = default;
	BlockLoader& operator=(BlockLoader&&) = default;
	BlockLoader& operator=(const BlockLoader&) = default;
};

}