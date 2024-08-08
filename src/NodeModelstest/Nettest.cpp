//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "NodeModels/NetModel.hpp"

using namespace node::model;
TEST(testNet, testCreate)
{
    NetModel net1{ 0 };
    NetNodeModel node1{ NetNodeId{1} };
    NetSegmentId segment_id = NetSegmentId{ 1 };
    node1.SetSegmentAt(NetNodeModel::ConnectedSegmentSide::east, segment_id);
    NetNodeModel node2{ NetNodeId{2} };
    node2.SetSegmentAt(NetNodeModel::ConnectedSegmentSide::west, segment_id);
    NetSegmentModel segment1{ NetSegmentId{1}, node1.GetId(), node2.GetId(),
        NetSegmentModel::NetSegmentOrientation::horizontal };
    
    auto node1_id = node1.GetId();
    auto node2_id = node2.GetId();
    auto segment1_id = segment1.GetId();

    net1.AddNetNode(std::move(node1));
    net1.AddNetNode(std::move(node2));
    net1.AddNetSegment(std::move(segment1));


    ASSERT_EQ(net1.GetNetNodes().size(), 2);
    EXPECT_TRUE(net1.GetNetNodeById(node1_id).has_value());
    EXPECT_TRUE(net1.GetNetNodeById(node2_id).has_value());
    EXPECT_FALSE(net1.GetNetNodeById(NetNodeId{ 3 }).has_value());

    ASSERT_EQ(net1.GetNetSegments().size(), 1);
    EXPECT_TRUE(net1.GetNetSegmentById(segment1_id));
    EXPECT_FALSE(net1.GetNetSegmentById(NetSegmentId{ 2 }).has_value());
}
