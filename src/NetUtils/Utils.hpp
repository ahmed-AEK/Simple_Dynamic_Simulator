#pragma once

#include "NodeModels/NetModel.hpp"

namespace node
{
namespace NetUtils
{


inline void connectSegementAndNodes(node::model::NetSegmentModel& segment,
    node::model::NetNodeModel& node1, node::model::NetNodeModel& node2)
{
    using namespace node;
    using namespace node::model;

    segment.m_firstNodeId = node1.GetId();
    segment.m_secondNodeId = node2.GetId();

    if (segment.m_orientation == NetSegmentOrientation::vertical)
    {
        if (node2.GetPosition().y < node1.GetPosition().y)
        {
            node1.SetSegmentAt(ConnectedSegmentSide::north, segment.GetId());
            node2.SetSegmentAt(ConnectedSegmentSide::south, segment.GetId());
        }
        else
        {
            node1.SetSegmentAt(ConnectedSegmentSide::south, segment.GetId());
            node2.SetSegmentAt(ConnectedSegmentSide::north, segment.GetId());
        }
    }
    else // horizontal
    {
        if (node2.GetPosition().x < node1.GetPosition().x)
        {
            node1.SetSegmentAt(ConnectedSegmentSide::west, segment.GetId());
            node2.SetSegmentAt(ConnectedSegmentSide::east, segment.GetId());
        }
        else
        {
            node1.SetSegmentAt(ConnectedSegmentSide::east, segment.GetId());
            node2.SetSegmentAt(ConnectedSegmentSide::west, segment.GetId());
        }
    }
}

struct StringNodeDescription
{
	model::Point point;
	model::NetSegmentOrientation next_segment_orientation;
};
struct StringNetDescription
{
	StringNetDescription(std::initializer_list<StringNodeDescription> elements)
		: nodes{ elements } {}
	std::vector<StringNodeDescription> nodes;
};

struct StringNetResult
{
	std::vector<model::NetNodeModel> nodes;
	std::vector<model::NetSegmentModel> segments;
};

StringNetResult CreateStringNet(const StringNetDescription& d);
}
}