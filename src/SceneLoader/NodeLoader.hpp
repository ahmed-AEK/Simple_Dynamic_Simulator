#pragma once

#include <memory>
#include "NodeModels/NodeScene.hpp"

namespace node::loader
{


class NodeLoader
{
public:
	virtual bool AddNode(const node::model::NodeModel& node) = 0;
	virtual bool DeleteNodeAndSockets(const node::model::id_int node_id) = 0;
	virtual bool UpdateNodePosition(node::model::id_int node_id, 
		const node::model::Point& position) = 0;
	virtual	std::optional<node::model::NodeModel> GetNode(node::model::id_int node_id) = 0;
	virtual bool UpdateNodeBounds(node::model::id_int node_id, 
		const node::model::Rect& bounds) = 0;

	virtual bool AddSocket(const node::model::NodeSocketModel& socket) = 0;
	virtual bool DeleteSocket(const node::model::NodeSocketId& socket_id) = 0;
	virtual bool UpdateSocketPosition(const node::model::NodeSocketId& socket_id,
		const node::model::Point& position) = 0;

	virtual node::model::id_int GetNextNodeIdx() = 0;
	virtual std::vector<node::model::NodeModel> GetNodes() = 0;

	virtual ~NodeLoader() = default;
	NodeLoader() = default;
	NodeLoader(NodeLoader&&) = default;
	NodeLoader(const NodeLoader&) = default;
	NodeLoader& operator=(NodeLoader&&) = default;
	NodeLoader& operator=(const NodeLoader&) = default;
};

}