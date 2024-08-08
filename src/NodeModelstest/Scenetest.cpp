//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "NodeModels/NodeScene.hpp"

using namespace node::model;
TEST(testScene, testCreate)
{
	NodeSceneModel scene;
	BlockId block_id = BlockId{ 1 };
	SocketId sock_id = SocketId{ 2 };
	BlockModel node1{ block_id, Rect{1,1,10,10} };
	BlockSocketModel sock1{ BlockSocketModel::SocketType::input, sock_id };

	node1.AddSocket(std::move(sock1));
	scene.AddBlock(std::move(node1));

	ASSERT_EQ(scene.GetBlocks().size(), 1);
	EXPECT_TRUE(scene.GetBlockById(block_id) != std::nullopt);
	EXPECT_TRUE(scene.GetBlockById(BlockId{ block_id.value + 1 }) == std::nullopt);
	EXPECT_TRUE(scene.GetBlockById(block_id)->get().GetSocketById(sock_id).has_value());
	EXPECT_FALSE(scene.GetBlockById(block_id)->get().GetSocketById(SocketId{ sock_id.value + 1 }).has_value());
}
