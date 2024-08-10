#include "NetModel.hpp"
#include <cassert>
#include <algorithm>

std::optional<node::model::NetSegmentId> node::model::NetNodeModel::GetSegmentAt(const ConnectedSegmentSide side)
{
	if (!m_valid_sides[static_cast<size_t>(side)])
	{
		return std::nullopt;
	}
	
	return m_segmentIds[static_cast<size_t>(side)];
}

void node::model::NetNodeModel::SetSegmentAt(const ConnectedSegmentSide side, const std::optional<NetSegmentId> segment)
{
	if (!segment)
	{
		m_valid_sides[static_cast<size_t>(side)] = false;
		return;
	}
	m_valid_sides[static_cast<size_t>(side)] = true;
	m_segmentIds[static_cast<size_t>(side)] = *segment;
}
