//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "GraphicsScene/NetObject.hpp"

bool operator==(const SDL_Rect& in1, const SDL_Rect& in2)
{
    return in1.x == in2.x && in1.y == in2.y && in1.w == in2.w && in1.h == in2.h;
}

TEST(testNetSegment, testCreate)
{
    node::NetNode node1{ {0,0},  nullptr };
    node::NetNode node2{ {0,1},  nullptr };
    node::NetSegment segment1 = node::NetSegment(node::NetOrientation::Vertical, &node1, &node2);
    
    node::NetNode node3{ {0,0}, nullptr };
    node::NetNode node4{ {1,0}, nullptr };
    node::NetSegment segment2 = node::NetSegment(node::NetOrientation::Horizontal, &node3, &node4);

    node::NetSegment segment3 = node::NetSegment(node::NetOrientation::Vertical, nullptr, nullptr);

    EXPECT_EQ(segment1.getOrientation(), node::NetOrientation::Vertical);
    EXPECT_EQ(segment2.getOrientation(), node::NetOrientation::Horizontal);
    EXPECT_EQ(segment1.getStartNode(), &node1);
    EXPECT_EQ(segment1.getEndNode(), &node2);
    EXPECT_EQ(segment3.getStartNode(), nullptr);
    
    SDL_Rect rect1 = SDL_Rect{ -5, 0, 10, 1 };
    SDL_Rect rect2 = SDL_Rect{ 0, -5, 1, 10 };
    EXPECT_EQ(segment1.GetSpaceRect(), rect1);
    EXPECT_EQ(segment2.GetSpaceRect(), rect2);

    EXPECT_EQ(node1.getSegment(node::NetSide::South), &segment1);
    EXPECT_EQ(node1.getSegment(node::NetSide::North), nullptr);
    EXPECT_EQ(node2.getSegment(node::NetSide::North), &segment1);
    EXPECT_EQ(node2.getSegment(node::NetSide::South), nullptr);
    EXPECT_EQ(node3.getSegment(node::NetSide::East), &segment2);
    EXPECT_EQ(node3.getSegment(node::NetSide::West), nullptr);
    EXPECT_EQ(node4.getSegment(node::NetSide::West), &segment2);
    EXPECT_EQ(node4.getSegment(node::NetSide::East), nullptr);
}

TEST(testNetSegment, testMoveNodeInLine)
{
    node::NetNode node1{ {0,0},  nullptr };
    node::NetNode node2{ {0,1},  nullptr };
    node::NetSegment segment1 = node::NetSegment(node::NetOrientation::Vertical, &node1, &node2);

    SDL_Rect rect1 = segment1.GetSpaceRect();
    node1.setCenter({ 2,2 });
    node2.setCenter({ 2,3 });
    node2.UpdateConnectedSegments();

    SDL_Rect rect2 = segment1.GetSpaceRect();

    node1.setCenter({ 2,0 });
    node2.setCenter({ 2,4 });
    node1.UpdateConnectedSegments();
    SDL_Rect rect3 = segment1.GetSpaceRect();

    SDL_Rect expected_rect1{ -5,0,10,1 };
    SDL_Rect expected_rect2{ -3,2,10,1 };
    SDL_Rect expected_rect3{ -3,0,10,4 };

    EXPECT_EQ(rect1, expected_rect1);
    EXPECT_EQ(rect2, expected_rect2);
    EXPECT_EQ(rect3, expected_rect3);

    
}
