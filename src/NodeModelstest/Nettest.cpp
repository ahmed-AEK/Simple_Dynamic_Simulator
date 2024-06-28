//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "NodeModels/NetModel.hpp"

using namespace node::model;
TEST(testNet, testCreate)
{
    NetModel net1{ 0 };
    NetNodeModel node1{ 1 };
    id_int segment_id = 1;
    node1.m_eastSegmentId = segment_id;
    NetNodeModel node2{ 2 };
    node2.m_westSegmentId = segment_id;
    NetSegmentModel segment1{ 1, node1.GetId(), node2.GetId(), NetSegmentOrientation::horizontal };

    net1.AddNetNode(node1);
    net1.AddNetNode(node2);
    net1.AddNetSegment(segment1);

    ASSERT_EQ(net1.GetNetNodes().size(), 2);
    EXPECT_TRUE(net1.GetNetNodeById(node1.GetId()).has_value());
    EXPECT_TRUE(net1.GetNetNodeById(node2.GetId()).has_value());
    EXPECT_FALSE(net1.GetNetNodeById(3).has_value());

    ASSERT_EQ(net1.GetNetSegments().size(), 1);
    EXPECT_TRUE(net1.GetNetSegmentById(segment1.GetId()));
    EXPECT_FALSE(net1.GetNetSegmentById(2).has_value());
}