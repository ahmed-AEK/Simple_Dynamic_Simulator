//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "NodeModels/NetModel.hpp"
#include "NodeModels/NodeScene.hpp"

using namespace node::model;
TEST(testNet, testCreate)
{
    NodeSceneModel scene;
    NetNodeModel node1{ NetNodeId{1} };
    NetSegmentId segment_id = NetSegmentId{ 1 };
    node1.SetSegmentAt(ConnectedSegmentSide::east, segment_id);
    NetNodeModel node2{ NetNodeId{2} };
    node2.SetSegmentAt(ConnectedSegmentSide::west, segment_id);
    NetSegmentModel segment1{ NetSegmentId{1}, node1.GetId(), node2.GetId(),
        NetSegmentOrientation::horizontal };
    
    auto node1_id = node1.GetId();
    auto node2_id = node2.GetId();
    auto segment1_id = segment1.GetId();

    scene.AddNetNode(std::move(node1));
    scene.AddNetNode(std::move(node2));
    scene.AddNetSegment(std::move(segment1));


    ASSERT_EQ(scene.GetNetNodes().size(), 2);
    EXPECT_TRUE(scene.GetNetNodeById(node1_id));
    EXPECT_TRUE(scene.GetNetNodeById(node2_id));
    EXPECT_FALSE(scene.GetNetNodeById(NetNodeId{ 3 }));

    ASSERT_EQ(scene.GetNetSegments().size(), 1);
    EXPECT_TRUE(scene.GetNetSegmentById(segment1_id));
    EXPECT_FALSE(scene.GetNetSegmentById(NetSegmentId{ 2 }));
}
