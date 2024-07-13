#pragma once

#include "NodeModels/NodeScene.hpp"

namespace node::loader
{


class NodeLoader
{
public:
	virtual bool AddNode(const node::model::BlockModelPtr& node) = 0;
	virtual bool DeleteNodeAndSockets(const node::model::id_int node_id) = 0;
	virtual bool UpdateNodePosition(node::model::id_int node_id, 
		const node::model::Point& position) = 0;
	virtual	std::shared_ptr<node::model::BlockModel>
		GetNode(node::model::id_int node_id) = 0;
	virtual bool UpdateNodeBounds(node::model::id_int node_id, 
		const node::model::Rect& bounds) = 0;

	virtual bool AddSocket(const node::model::BlockSocketModel& socket) = 0;
	virtual bool DeleteSocket(const node::model::BlockSocketId& socket_id) = 0;
	virtual bool UpdateSocketPosition(const node::model::BlockSocketId& socket_id,
		const node::model::Point& position) = 0;

	virtual node::model::id_int GetNextNodeIdx() = 0;
	virtual std::vector<std::shared_ptr<node::model::BlockModel>> GetNodes() = 0;

	virtual ~NodeLoader() = default;
	NodeLoader() = default;
	NodeLoader(NodeLoader&&) = default;
	NodeLoader(const NodeLoader&) = default;
	NodeLoader& operator=(NodeLoader&&) = default;
	NodeLoader& operator=(const NodeLoader&) = default;
};

}