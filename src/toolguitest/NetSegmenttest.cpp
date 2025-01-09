//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "GraphicsScene/NetObject.hpp"

using namespace node;

TEST(testNetSegment, testCreate)
{
    node::NetNode node1{ {0,0} };
    node::NetNode node2{ {0,1} };
    node::NetSegment segment1 = node::NetSegment(model::NetSegmentOrientation::vertical, &node1, &node2);
    
    node::NetNode node3{ {0,0} };
    node::NetNode node4{ {1,0} };
    node::NetSegment segment2 = node::NetSegment(model::NetSegmentOrientation::horizontal, &node3, &node4);

    node::NetSegment segment3 = node::NetSegment(model::NetSegmentOrientation::vertical, nullptr, nullptr);

    EXPECT_EQ(segment1.GetOrientation(), model::NetSegmentOrientation::vertical);
    EXPECT_EQ(segment2.GetOrientation(), model::NetSegmentOrientation::horizontal);
    EXPECT_EQ(segment1.getStartNode(), &node1);
    EXPECT_EQ(segment1.getEndNode(), &node2);
    EXPECT_EQ(segment3.getStartNode(), nullptr);
    
    model::Rect rect1{ -5, 0, 10, 1 };
    model::Rect rect2{ 0, -5, 1, 10 };
    EXPECT_EQ(segment1.GetSceneRect(), rect1);
    EXPECT_EQ(segment2.GetSceneRect(), rect2);

    EXPECT_EQ(node1.getSegment(model::ConnectedSegmentSide::south), &segment1);
    EXPECT_EQ(node1.getSegment(model::ConnectedSegmentSide::north), nullptr);
    EXPECT_EQ(node2.getSegment(model::ConnectedSegmentSide::north), &segment1);
    EXPECT_EQ(node2.getSegment(model::ConnectedSegmentSide::south), nullptr);
    EXPECT_EQ(node3.getSegment(model::ConnectedSegmentSide::east), &segment2);
    EXPECT_EQ(node3.getSegment(model::ConnectedSegmentSide::west), nullptr);
    EXPECT_EQ(node4.getSegment(model::ConnectedSegmentSide::west), &segment2);
    EXPECT_EQ(node4.getSegment(model::ConnectedSegmentSide::east), nullptr);
}

TEST(testNetSegment, testMoveNodeInLine)
{
    node::NetNode node1{ {0,0} };
    node::NetNode node2{ {0,1} };
    node::NetSegment segment1 = node::NetSegment(model::NetSegmentOrientation::vertical, &node1, &node2);

    model::Rect rect1 = segment1.GetSceneRect();
    node1.setCenter({ 2,2 });
    node2.setCenter({ 2,3 });
    node2.UpdateConnectedSegments();

    model::Rect rect2 = segment1.GetSceneRect();

    node1.setCenter({ 2,0 });
    node2.setCenter({ 2,4 });
    node1.UpdateConnectedSegments();
    model::Rect rect3 = segment1.GetSceneRect();

    model::Rect expected_rect1{ -5,0,10,1 };
    model::Rect expected_rect2{ -3,2,10,1 };
    model::Rect expected_rect3{ -3,0,10,4 };

    EXPECT_EQ(rect1, expected_rect1);
    EXPECT_EQ(rect2, expected_rect2);
    EXPECT_EQ(rect3, expected_rect3);

    
}
