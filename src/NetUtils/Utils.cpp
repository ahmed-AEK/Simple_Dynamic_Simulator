#include "Utils.hpp"
#include <cassert>

node::NetUtils::StringNetResult node::NetUtils::CreateStringNet(const StringNetDescription& d)
{
    using namespace node::model;
    assert(d.nodes.size());

    StringNetResult result;
    for (size_t i = 0; i < d.nodes.size(); i++)
    {
        result.nodes.push_back(NetNodeModel{ NetNodeId{static_cast<id_int>(i)}, d.nodes[i].point});
    }
    for (size_t i = 1; i < d.nodes.size(); i++)
    {
        result.segments.push_back(NetSegmentModel{ NetSegmentId{static_cast<id_int>(i - 1)}, result.nodes[i - 1].GetId(), result.nodes[i].GetId(), d.nodes[i-1].next_segment_orientation });
        connectSegementAndNodes(result.segments.back(), result.nodes[i - 1], result.nodes[i]);

        // make sure it is actually vertical/horizontal
        if (d.nodes[i - 1].next_segment_orientation == NetSegmentOrientation::vertical)
        {
            assert(d.nodes[i - 1].point.x == d.nodes[i].point.x);
        }
        else
        {
            assert(d.nodes[i - 1].point.y == d.nodes[i].point.y);
        }
    }
    return result;
}
