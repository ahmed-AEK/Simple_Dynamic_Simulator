
using namespace node::loader;
using namespace node::model;
using namespace node;


TEST(testSceneLoader, testSaveLoadId)
{
	NodeSceneModel scene;
	auto subscene_id = SubSceneId{ 2 };
	scene.SetSubSceneId(subscene_id);
	SubSceneId parent_subscene_id{ 0 };


	SQLSceneLoader loader(":memory:");

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetSubSceneId(), subscene_id);
}


TEST(testSceneLoader, testSaveLoadBlock)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	BlockModel original_block{ block_id, BlockType::Functional, Rect{1,1,10,10} };

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(BlockModel{ original_block });

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 1);
}

TEST(testSceneLoader, DISABLED_testSaveModifyLoadBlock)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	BlockModel block1{ block_id, BlockType::Functional, Rect{1,1,10,10} };

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(std::move(block1));

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto nodeLoader = loader.GetBlockLoader(subscene_id);
	ASSERT_TRUE(nodeLoader);

	auto second_rect = node::model::Rect{ 2,2,5,5 };
	nodeLoader->UpdateBlockBounds(block_id, second_rect);

	auto loaded_scene = loader.Load(subscene_id);
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 1);
	auto rect = loaded_scene->GetBlocks()[0].GetBounds();
	EXPECT_EQ(rect, second_rect);

}

TEST(testSceneLoader, DISABLED_testSaveDeleteLoadNode)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };

	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	BlockModel block1{ block_id, BlockType::Functional, Rect{1,1,10,10} };

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(std::move(block1));

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);
	auto nodeLoader = loader.GetBlockLoader(subscene_id);
	ASSERT_TRUE(nodeLoader);
	auto result2 = nodeLoader->DeleteBlockAndSockets(block_id);

	auto loaded_scene = loader.Load(subscene_id);
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_TRUE(result2);
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 0);
}

TEST(testSceneLoader, DISABLED_testNextIndex)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };

	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	BlockModel block1{ block_id, BlockType::Functional, Rect{1,1,10,10} };

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(std::move(block1));

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto nodeLoader = loader.GetBlockLoader(subscene_id);
	ASSERT_TRUE(nodeLoader);
	EXPECT_EQ(nodeLoader->GetNextBlockId(), BlockId{ 2 });

}

TEST(testSceneLoader, DISABLED_testNextIndexEmpty)
{
	NodeSceneModel scene;

	SQLSceneLoader loader(":memory:");
	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto nodeLoader = loader.GetBlockLoader(subscene_id);
	ASSERT_TRUE(nodeLoader);
	EXPECT_EQ(nodeLoader->GetNextBlockId(), BlockId{1});

}