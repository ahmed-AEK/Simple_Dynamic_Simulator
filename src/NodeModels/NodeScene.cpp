#include "NodeScene.hpp"
#include <cassert>
#include <algorithm>

std::optional<node::model::NetModelRef> node::model::NodeSceneModel::GetNetById(NetId id)
{
	auto iter = std::find_if(m_nets.begin(), m_nets.end(),
		[id](const auto& net) {return id == net.GetId(); });
	if (iter != m_nets.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

void node::model::NodeSceneModel::RemoveNetById(NetId id)
{
	auto it = std::find_if(m_nets.begin(), m_nets.end(),
		[&](const auto& net) { return net.GetId() == id; });

	assert(it != m_nets.end());
	m_nets.erase(it);
}



std::optional<node::model::BlockModelRef> node::model::NodeSceneModel::GetBlockById(const BlockId& id)
{
	auto iter = std::find_if(m_blocks.begin(), m_blocks.end(),
		[id](const BlockModel& node) {return id == node.GetId(); });
	if (iter != m_blocks.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

void node::model::NodeSceneModel::RemoveBlockById(const BlockId& id) {
	auto it = std::find_if(m_blocks.begin(), m_blocks.end(),
		[&](const BlockModel& node) { return node.GetId() == id; });

	assert(it != m_blocks.end());
	m_blocks.erase(it);
}