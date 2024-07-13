//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "NodeModels/NodeScene.hpp"

using namespace node::model;
TEST(testScene, testCreate)
{
	NodeSceneModel scene;
	id_int node_id = 1;
	id_int sock_id = 2;
	std::shared_ptr<BlockModel> node1 = std::make_shared<BlockModel>(node_id, Rect{1,1,10,10});
	BlockSocketModel sock1{ BlockSocketModel::SocketType::input, {sock_id,node1->GetId()} };

	node1->AddSocket(std::move(sock1));
	scene.AddBlock(std::move(node1));

	ASSERT_EQ(scene.GetBlocks().size(), 1);
	EXPECT_TRUE(scene.GetBlockById(node_id));
	EXPECT_FALSE(scene.GetBlockById(node_id + 1));
	EXPECT_TRUE(scene.GetBlockById(node_id)->GetSocketById(sock_id, BlockSocketModel::SocketType::input).has_value());
	EXPECT_FALSE(scene.GetBlockById(node_id)->GetSocketById(sock_id + 1, BlockSocketModel::SocketType::input).has_value());
}
