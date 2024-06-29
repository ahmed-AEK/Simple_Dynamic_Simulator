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

std::optional<node::model::id_int> node::model::NetNodeModel::GetSegmentAt(const ConnectedSegmentSide side)
{
	switch (side)
	{
	case ConnectedSegmentSide::north:
		return m_northSegmentId;
	case ConnectedSegmentSide::south:
		return m_southSegmentId;
	case ConnectedSegmentSide::west:
		return m_westSegmentId;
	case ConnectedSegmentSide::east:
		return m_eastSegmentId;
	default:
		return std::nullopt;
	}
}

void node::model::NetNodeModel::SetSegmentAt(const ConnectedSegmentSide side, const std::optional<id_int> segment)
{
	switch (side)
	{
	case ConnectedSegmentSide::north:
		m_northSegmentId = segment;
		break;
	case ConnectedSegmentSide::south:
		m_southSegmentId = segment;
		break;
	case ConnectedSegmentSide::west:
		m_westSegmentId = segment;
		break;
	case ConnectedSegmentSide::east:
		m_eastSegmentId = segment;
		break;
	}
}
