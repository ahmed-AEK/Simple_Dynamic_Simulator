//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "SceneLoader/SQLSceneLoader.hpp"

using namespace node::loader;
using namespace node::model;

TEST(testScene, testSaveLoadNode)
{
	NodeSceneModel scene;
	id_int node_id = 1;
	std::shared_ptr<BlockModel> node1 = std::make_shared<BlockModel>(node_id, Rect{1,1,10,10});

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(std::move(node1));

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 1);

}

TEST(testScene, testSaveModifyLoadNode)
{
	NodeSceneModel scene;
	id_int node_id = 1;
	BlockModelPtr node1 = std::make_shared<BlockModel>(node_id, Rect{1,1,10,10});

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(std::move(node1));

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto nodeLoader = loader.GetNodeLoader();
	auto second_rect = node::model::Rect{ 2,2,5,5 };
	nodeLoader->UpdateNodeBounds(node_id, second_rect);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 1);
	auto rect = loaded_scene->GetBlocks()[0]->GetBounds();
	EXPECT_EQ(rect, second_rect);

}

TEST(testScene, testSaveDeleteLoadNode)
{
	NodeSceneModel scene;
	id_int node_id = 1;
	BlockModelPtr node1 = std::make_shared<BlockModel>(node_id, Rect{1,1,10,10});

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(std::move(node1));

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);
	auto nodeLoader = loader.GetNodeLoader();
	
	auto result2 = nodeLoader->DeleteNodeAndSockets(node_id);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_TRUE(result2);
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 0);
}

TEST(testScene, testNextIndex)
{
	NodeSceneModel scene;
	id_int node_id = 1;
	BlockModelPtr node1 = std::make_shared<BlockModel>(node_id, Rect{1,1,10,10});

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(std::move(node1));

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto nodeLoader = loader.GetNodeLoader();
	EXPECT_EQ(nodeLoader->GetNextNodeIdx(), 2);

}

TEST(testScene, testNextIndexEmpty)
{
	NodeSceneModel scene;

	SQLSceneLoader loader(":memory:");

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto nodeLoader = loader.GetNodeLoader();
	EXPECT_EQ(nodeLoader->GetNextNodeIdx(), 1);

}