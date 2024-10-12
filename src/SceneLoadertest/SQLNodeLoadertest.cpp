//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "SceneLoader/SQLNodeLoader.hpp"
#include "SceneLoader/SQLSceneLoader.hpp"

using namespace node::loader;
using namespace node::model;

TEST(testBlockLoader, testSaveLoadBlockIdAndBoundsAndOrientation)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, block_rect };
	BlockOrientation orienation = BlockOrientation::LeftToRight;
	scene.AddBlock(BlockModel{ original_block });

	SQLSceneLoader loader(":memory:");

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 1);
	auto loaded_block = loaded_scene.value().GetBlockById(block_id);
	ASSERT_TRUE(loaded_block);
	EXPECT_EQ(loaded_block->get().GetBounds(), block_rect);
	EXPECT_EQ(loaded_block->get().GetOrienation(), orienation);
}

TEST(testBlockLoader, testSaveLoadBlockIdAndBoundsAndOrientation2)
{
	NodeSceneModel scene;

	{
		BlockId block_id{ 1 };
		Rect block_rect{ 1,1,10,10 };
		BlockModel original_block{ block_id, block_rect };
		scene.AddBlock(BlockModel{ original_block });
	}

	BlockId block_id{ 2 };
	Rect block_rect{ 20,30,40,50 };
	BlockOrientation orienation = BlockOrientation::TopToBottom;
	BlockModel original_block{ block_id, block_rect, orienation };
	scene.AddBlock(BlockModel{ original_block });

	SQLSceneLoader loader(":memory:");

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 2);
	auto loaded_block = loaded_scene.value().GetBlockById(block_id);
	ASSERT_TRUE(loaded_block);
	EXPECT_EQ(loaded_block->get().GetBounds(), block_rect);
	EXPECT_EQ(loaded_block->get().GetOrienation(), orienation);
}

TEST(testBlockLoader, testSaveLoadBlockClassAndProperties)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, block_rect };
	std::string_view property_name = "Multiplier";
	std::string class_name{ "test1" };
	original_block.GetProperties().push_back(BlockProperty{ std::string{property_name}, BlockPropertyType::FloatNumber, 1.0});
	original_block.SetClass(class_name);

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(BlockModel{ original_block });

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	auto loaded_block = loaded_scene.value().GetBlockById(block_id);
	ASSERT_TRUE(loaded_block);
	EXPECT_EQ(loaded_block->get().GetClass(), class_name);
	
	auto&& loaded_properties = loaded_block->get().GetProperties();
	ASSERT_EQ(loaded_properties.size(), 1);
	EXPECT_EQ(loaded_properties[0].name, property_name);
	EXPECT_EQ(loaded_properties[0].type, BlockPropertyType::FloatNumber);
	ASSERT_TRUE(std::holds_alternative<double>(loaded_properties[0].prop));
	EXPECT_EQ(std::get<double>(loaded_properties[0].prop), 1.0);
}

TEST(testBlockLoader, testSaveLoadBlockClassAndProperties2)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, block_rect };
	std::string_view property_name = "Value";
	std::string_view property2_name = "Value2";
	std::string class_name{ "test2" };
	original_block.GetProperties().push_back(BlockProperty{ std::string{property_name}, BlockPropertyType::Boolean, true });
	original_block.GetProperties().push_back(BlockProperty{ std::string{property2_name}, BlockPropertyType::Integer, static_cast<int64_t>(2) });
	original_block.SetClass(class_name);

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(BlockModel{ original_block });

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	auto loaded_block = loaded_scene.value().GetBlockById(block_id);
	ASSERT_TRUE(loaded_block);
	EXPECT_EQ(loaded_block->get().GetClass(), class_name);

	auto&& loaded_properties = loaded_block->get().GetProperties();
	ASSERT_EQ(loaded_properties.size(), 2);

	EXPECT_EQ(loaded_properties[0].name, property_name);
	EXPECT_EQ(loaded_properties[0].type, BlockPropertyType::Boolean);
	ASSERT_TRUE(std::holds_alternative<bool>(loaded_properties[0].prop));
	EXPECT_EQ(std::get<bool>(loaded_properties[0].prop), true);

	EXPECT_EQ(loaded_properties[1].name, property2_name);
	EXPECT_EQ(loaded_properties[1].type, BlockPropertyType::Integer);
	ASSERT_TRUE(std::holds_alternative<int64_t>(loaded_properties[1].prop));
	EXPECT_EQ(std::get<int64_t>(loaded_properties[1].prop), 2);
}


TEST(testBlockLoader, testSaveLoadBlockStylerAndProperties)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, block_rect };
	std::string property_name = "Multiplier";
	std::string property_value = "Value1";	
	std::string styler_name{ "test1" };
	BlockStyleProperties properties{ {{property_name,property_value}} };
	original_block.SetStylerProperties(properties);
	original_block.SetStyler(styler_name);

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(BlockModel{ original_block });

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	auto loaded_block = loaded_scene.value().GetBlockById(block_id);
	ASSERT_TRUE(loaded_block);
	EXPECT_EQ(loaded_block->get().GetStyler(), styler_name);

	auto&& loaded_properties = loaded_block->get().GetStylerProperties();
	ASSERT_EQ(loaded_properties.properties.size(), 1);
	ASSERT_TRUE(loaded_properties.properties.contains(property_name));
	EXPECT_EQ(loaded_properties.properties.find(property_name)->second, property_value);
}


TEST(testBlockLoader, testSaveLoadBlockStylerAndProperties2)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, block_rect };
	std::string property_name = "Multiplier";
	std::string property_value = "Value1";
	std::string styler_name{ "test1" };
	std::string property2_name = "Multiplier2";
	std::string property2_value = "Value12";

	BlockStyleProperties properties{ {{property_name,property_value}, {property2_name, property2_value}} };
	original_block.SetStylerProperties(properties);
	original_block.SetStyler(styler_name);

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(BlockModel{ original_block });

	auto result = loader.Save(scene);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load();
	ASSERT_TRUE(loaded_scene.has_value());
	auto loaded_block = loaded_scene.value().GetBlockById(block_id);
	ASSERT_TRUE(loaded_block);
	EXPECT_EQ(loaded_block->get().GetStyler(), styler_name);

	auto&& loaded_properties = loaded_block->get().GetStylerProperties();
	ASSERT_EQ(loaded_properties.properties.size(), 2);
	ASSERT_TRUE(loaded_properties.properties.contains(property_name));
	EXPECT_EQ(loaded_properties.properties.find(property_name)->second, property_value);
	ASSERT_TRUE(loaded_properties.properties.contains(property2_name));
	EXPECT_EQ(loaded_properties.properties.find(property2_name)->second, property2_value);
}
