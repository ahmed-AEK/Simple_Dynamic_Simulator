//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "NodeModels/NodeModel.hpp"

using namespace node::model;
TEST(testNode, testCreate)
{
	NodeModel node1{ 1 , {10,10,10,10} };
	NodeSocketModel socket1{ NodeSocketType::input, {2, node1.GetId()}, {0,5} };
	node1.AddSocket(socket1);

	ASSERT_EQ(node1.GetSockets().size(), 1);
	EXPECT_TRUE(node1.GetSocketById(socket1.GetId().m_Id).has_value());
	EXPECT_FALSE(node1.GetSocketById(1).has_value());
}
