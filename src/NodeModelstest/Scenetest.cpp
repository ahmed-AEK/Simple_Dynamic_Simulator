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
	NodeModel node1{ node_id, {1,1,10,10} };
	NodeSocketModel sock1{ NodeSocketType::input, {sock_id,node1.GetId()} };

	node1.AddSocket(std::move(sock1));
	scene.AddNode(std::move(node1));

	ASSERT_EQ(scene.GetNodes().size(), 1);
	EXPECT_TRUE(scene.GetNodeById(node_id).has_value());
	EXPECT_FALSE(scene.GetNodeById(node_id + 1).has_value());
	EXPECT_TRUE(scene.GetNodeById(node_id).value().get().GetSocketById(sock_id).has_value());
	EXPECT_FALSE(scene.GetNodeById(node_id).value().get().GetSocketById(sock_id + 1).has_value());
}
