//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "SceneLoader/SQLNodeLoader.hpp"
#include "SceneLoader/SQLSceneLoader.hpp"

using namespace node::loader;
using namespace node::model;

TEST(testSQLNodeLoader, testChangeNodePosition)
{
	std::string db_name = ":memory:";
	id_int node_id = 5;
	SQLSceneLoader sceneLoader{ db_name };
	std::shared_ptr<NodeLoader> nodeLoader = sceneLoader.GetNodeLoader();
	Point old_position = { 1,1 };
	Point new_position = { 5,5 };
	{
		NodeModelPtr node1 = std::make_shared<NodeModel>(node_id, Rect{ old_position,10,10 });
		NodeSceneModel scene;
		scene.AddNode(node1);
		sceneLoader.Save(scene);
	}

	{
		NodeModelPtr node2 = nodeLoader->GetNode(node_id);
		ASSERT_TRUE(node2);
		EXPECT_EQ(node2->GetId(), node_id);
		EXPECT_EQ(node2->GetPosition(), old_position);
		node2->SetPosition(new_position);
	}

	NodeModelPtr node3 = nodeLoader->GetNode(node_id);
	EXPECT_EQ(node3->GetPosition(), new_position);

}
