//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "NodeModels/NetModel.hpp"

using namespace node::model;
TEST(testNetSegment, testCreate)
{
    node::model::NetSegmentModel segment{ NetSegmentId{1},NetNodeId{1},NetNodeId{2} ,
        NetSegmentModel::NetSegmentOrientation::horizontal};
    ASSERT_EQ(segment.m_firstNodeId, NetNodeId{ 1 });
    ASSERT_EQ(segment.m_secondNodeId, NetNodeId{ 2 });
    ASSERT_EQ(segment.GetId(), NetSegmentId{ 1 });
}
