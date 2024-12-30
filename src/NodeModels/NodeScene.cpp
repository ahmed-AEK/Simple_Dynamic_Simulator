#include "NodeScene.hpp"
#include <cassert>
#include <algorithm>


node::model::BlockModel* node::model::NodeSceneModel::GetBlockById(const BlockId& id)
{
	auto iter = std::find_if(m_blocks.begin(), m_blocks.end(),
		[id](const BlockModel& node) {return id == node.GetId(); });
	if (iter != m_blocks.end())
	{
		return &*iter;
	}
	return nullptr;
}

const node::model::BlockModel* node::model::NodeSceneModel::GetBlockById(const BlockId& id) const
{
	auto iter = std::find_if(m_blocks.begin(), m_blocks.end(),
		[id](const BlockModel& node) {return id == node.GetId(); });
	if (iter != m_blocks.end())
	{
		return &*iter;
	}
	return nullptr;
}

void node::model::NodeSceneModel::RemoveBlockById(const BlockId& id) {
	auto it = std::find_if(m_blocks.begin(), m_blocks.end(),
		[&](const BlockModel& node) { return node.GetId() == id; });

	assert(it != m_blocks.end());
	m_blocks.erase(it);
}

void node::model::NodeSceneModel::RemoveNetNodeById(const NetNodeId& id)
{
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
		[&](const NetNodeModel& node) { return node.GetId() == id; });

	assert(it != m_nodes.end());
	m_nodes.erase(it);
}

const node::model::NetNodeModel* node::model::NodeSceneModel::GetNetNodeById(const NetNodeId& id) const
{
	auto iter = std::find_if(m_nodes.begin(), m_nodes.end(),
		[id](const NetNodeModel& node) {return id == node.GetId(); });
	if (iter != m_nodes.end())
	{
		return &*iter;
	}
	return nullptr;
}

node::model::NetNodeModel* node::model::NodeSceneModel::GetNetNodeById(const NetNodeId& id)
{
	auto iter = std::find_if(m_nodes.begin(), m_nodes.end(),
		[id](const NetNodeModel& node) {return id == node.GetId(); });
	if (iter != m_nodes.end())
	{
		return &*iter;
	}
	return nullptr;
}

void node::model::NodeSceneModel::RemoveNetSegmentById(const NetSegmentId& id)
{
	auto it = std::find_if(m_segments.begin(), m_segments.end(),
		[&](const NetSegmentModel& segment) { return segment.GetId() == id; });

	assert(it != m_segments.end());
	m_segments.erase(it);
}

const node::model::NetSegmentModel* node::model::NodeSceneModel::GetNetSegmentById(const NetSegmentId& id) const
{
	auto iter = std::find_if(m_segments.begin(), m_segments.end(),
		[id](const NetSegmentModel& segment) {return id == segment.GetId(); });
	if (iter != m_segments.end())
	{
		return &*iter;
	}
	return nullptr;
}

node::model::NetSegmentModel* node::model::NodeSceneModel::GetNetSegmentById(const NetSegmentId& id)
{
	auto iter = std::find_if(m_segments.begin(), m_segments.end(),
		[id](const NetSegmentModel& segment) {return id == segment.GetId(); });
	if (iter != m_segments.end())
	{
		return &*iter;
	}
	return nullptr;
}

void node::model::NodeSceneModel::RemoveSocketConnectionForSocket(const model::SocketUniqueId& socket)
{
	auto iter = std::find_if(m_SocketConnections.begin(), m_SocketConnections.end(),
		[&](const SocketNodeConnection& conn) {
			return conn.socketId == socket;
		});
	assert(iter != m_SocketConnections.end());
	if (iter != m_SocketConnections.end())
	{
		m_SocketConnections.erase(iter);
	}
}

const node::model::SocketNodeConnection* node::model::NodeSceneModel::GetSocketConnectionForSocket(const model::SocketUniqueId& socket_id) const
{
	auto iter = std::find_if(m_SocketConnections.begin(), m_SocketConnections.end(),
		[&](const SocketNodeConnection& conn) {
			return conn.socketId == socket_id;
		});
	if (iter != m_SocketConnections.end())
	{
		return &*iter;
	}
	return nullptr;
}

const node::model::SocketNodeConnection* node::model::NodeSceneModel::GetSocketConnectionForNode(const model::NetNodeId& node_id) const
{
	auto iter = std::find_if(m_SocketConnections.begin(), m_SocketConnections.end(),
		[&](const SocketNodeConnection& conn) {
			return conn.NodeId == node_id;
		});
	if (iter != m_SocketConnections.end())
	{
		return &*iter;
	}
	return nullptr;
}

node::model::SocketNodeConnection* node::model::NodeSceneModel::GetSocketConnectionForNode(const model::NetNodeId& node_id)
{
	auto iter = std::find_if(m_SocketConnections.begin(), m_SocketConnections.end(),
		[&](const SocketNodeConnection& conn) {
			return conn.NodeId == node_id;
		});
	if (iter != m_SocketConnections.end())
	{
		return &*iter;
	}
	return nullptr;
}
