#include "NetModel.hpp"
#include <cassert>

std::optional<std::reference_wrapper<node::model::NetNodeModel>> 
node::model::NetModel::GetNetNodeById(id_int id)
{
	auto iter = std::find_if(m_nodes.begin(), m_nodes.end(),
		[id](const NetNodeModel& node) {return id == node.GetId(); });
	if (iter != m_nodes.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

void node::model::NetModel::RemoveNetNodeById(id_int id)
{
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
		[&](const NetNodeModel& node) { return node.GetId() == id; });

	assert(it != m_nodes.end());
	m_nodes.erase(it);
}

std::optional<std::reference_wrapper<node::model::NetSegmentModel>> node::model::NetModel::GetNetSegmentById(id_int id)
{
	auto iter = std::find_if(m_segments.begin(), m_segments.end(),
		[id](const NetSegmentModel& segment) {return id == segment.GetId(); });
	if (iter != m_segments.end())
	{
		return { *iter };
	}
	return std::nullopt;
}

void node::model::NetModel::RemoveNetSegmentById(id_int id)
{
	auto it = std::find_if(m_segments.begin(), m_segments.end(),
		[&](const NetSegmentModel& segment) { return segment.GetId() == id; });

	assert(it != m_segments.end());
	m_segments.erase(it);
}