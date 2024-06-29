//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "NodeModels/NetModel.hpp"

using namespace node::model;
TEST(testNetSegment, testCreate)
{
    node::model::NetSegmentModel segment{ 1,1,2 , 
        NetSegmentModel::NetSegmentOrientation::horizontal};
    ASSERT_EQ(segment.m_firstNodeId, 1);
    ASSERT_EQ(segment.m_secondNodeId, 2);
    ASSERT_EQ(segment.GetId(), 1);
}
