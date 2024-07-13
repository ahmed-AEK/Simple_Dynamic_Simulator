//#pragma warning( push , 0)
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
//#pragma warning( pop ) 
#include "NodeModels/BlockModel.hpp"

using namespace node::model;

TEST(testNode, testCreate)
{
	BlockModel node1{ 1 , {10,10,10,10} };
}

TEST(testNode, testAddSocket)
{
	BlockModel node1{ 1 , {10,10,10,10} };
	BlockSocketModel socket1{ BlockSocketModel::SocketType::input, {2, node1.GetId()}, {0,5} };
	
	node1.AddSocket(socket1);

	ASSERT_EQ(node1.GetSockets(BlockSocketModel::SocketType::input).size(), 1);
	EXPECT_TRUE(node1.GetSocketById(socket1.GetId().m_Id, BlockSocketModel::SocketType::input).has_value());
	EXPECT_FALSE(node1.GetSocketById(1, BlockSocketModel::SocketType::input).has_value());

}
