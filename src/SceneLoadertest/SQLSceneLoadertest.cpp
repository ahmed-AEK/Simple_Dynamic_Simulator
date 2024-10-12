//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "SceneLoader/SQLSceneLoader.hpp"

using namespace node::loader;
using namespace node::model;

TEST(testSceneLoader, testSaveLoadBlock)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	BlockModel original_block{ block_id, Rect{1,1,10,10} };

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(BlockModel{ original_block });

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 1);
}

TEST(testSceneLoader, testSaveModifyLoadBlock)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	BlockModel block1{ block_id, Rect{1,1,10,10} };

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(std::move(block1));

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto nodeLoader = loader.GetBlockLoader();
	auto second_rect = node::model::Rect{ 2,2,5,5 };
	nodeLoader->UpdateBlockBounds(block_id, second_rect);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 1);
	auto rect = loaded_scene->GetBlocks()[0].GetBounds();
	EXPECT_EQ(rect, second_rect);

}

TEST(testSceneLoader, testSaveDeleteLoadNode)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	BlockModel block1{ block_id, Rect{1,1,10,10} };

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(std::move(block1));

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);
	auto nodeLoader = loader.GetBlockLoader();
	
	auto result2 = nodeLoader->DeleteBlockAndSockets(block_id);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_TRUE(result2);
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 0);
}

TEST(testSceneLoader, testNextIndex)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	BlockModel block1{ block_id, Rect{1,1,10,10} };

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(std::move(block1));

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto nodeLoader = loader.GetBlockLoader();
	EXPECT_EQ(nodeLoader->GetNextBlockId(), BlockId{ 2 });

}

TEST(testSceneLoader, testNextIndexEmpty)
{
	NodeSceneModel scene;

	SQLSceneLoader loader(":memory:");

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto nodeLoader = loader.GetBlockLoader();
	EXPECT_EQ(nodeLoader->GetNextBlockId(), BlockId{1});

}