#include "NodeScene.hpp"
#include <cassert>

std::optional<std::reference_wrapper<node::model::NetModel>> node::model::NodeSceneModel::GetNetById(id_int id)
{
	auto iter = std::find_if(m_nets.begin(), m_nets.end(),
		[id](const NetModel& net) {return id == net.GetId(); });
	if (iter != m_nets.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

void node::model::NodeSceneModel::RemoveNetById(id_int id)
{
	auto it = std::find_if(m_nets.begin(), m_nets.end(),
		[&](const NetModel& net) { return net.GetId() == id; });

	assert(it != m_nets.end());
	m_nets.erase(it);
}

std::shared_ptr<node::model::BlockModel>
node::model::NodeSceneModel::GetBlockById(const id_int id)
{
	auto iter = std::find_if(m_blocks.begin(), m_blocks.end(),
		[id](const BlockModelPtr& node) {return id == node->GetId(); });
	if (iter != m_blocks.end())
	{
		return *iter;
	}
	return {};
}

void node::model::NodeSceneModel::RemoveBlockById(id_int id) {
	auto it = std::find_if(m_blocks.begin(), m_blocks.end(),
		[&](const BlockModelPtr& node) { return node->GetId() == id; });

	assert(it != m_blocks.end());
	m_blocks.erase(it);
}