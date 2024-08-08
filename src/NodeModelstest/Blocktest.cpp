//#pragma warning( push , 0)
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
//#pragma warning( pop ) 
#include "NodeModels/BlockModel.hpp"

using namespace node::model;

TEST(testNode, testCreate)
{
	BlockModel node1{ BlockId{1} , Rect{10,10,10,10} };
}

TEST(testNode, testAddSocket)
{
	BlockModel node1{ BlockId{1} , Rect{10,10,10,10} };
	BlockSocketModel socket1{ BlockSocketModel::SocketType::input, SocketId{2}, { 0,5 }};
	
	node1.AddSocket(socket1);

	ASSERT_EQ(node1.GetSockets().size(), 1);
	EXPECT_TRUE(node1.GetSocketById(socket1.GetId()).has_value());
	EXPECT_FALSE(node1.GetSocketById(SocketId{ 1 }).has_value());

}