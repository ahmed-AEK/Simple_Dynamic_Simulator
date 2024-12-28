//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "SceneLoader/SQLBlockLoader.hpp"
#include "SceneLoader/SQLSceneLoader.hpp"

using namespace node::loader;
using namespace node::model;
using namespace node;

TEST(testBlockLoader, testSaveLoadBlockIdAndBoundsAndOrientation)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };
	Rect block_rect{ 1,1,10,10 };

	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	BlockModel original_block{ block_id, BlockType::Functional, block_rect };
	BlockOrientation orienation = BlockOrientation::LeftToRight;
	scene.AddBlock(BlockModel{ original_block });

	SQLSceneLoader loader(":memory:");

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
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
		BlockModel original_block{ block_id, BlockType::Functional, block_rect };
		scene.AddBlock(BlockModel{ original_block });
	}

	BlockId block_id{ 2 };

	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	Rect block_rect{ 20,30,40,50 };
	BlockOrientation orienation = BlockOrientation::TopToBottom;
	BlockModel original_block{ block_id, BlockType::Functional, block_rect, orienation };
	scene.AddBlock(BlockModel{ original_block });

	SQLSceneLoader loader(":memory:");

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
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

	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, model::BlockType::Functional, block_rect };
	std::string_view property_name = "Multiplier";
	std::string class_name{ "test1" };
	model::FunctionalBlockData block_data;
	block_data.block_class = class_name;
	block_data.properties.push_back(BlockProperty{ std::string{property_name}, BlockPropertyType::FloatNumber, 1.0 });
	scene.GetFunctionalBlocksManager().SetDataForId(block_id, std::move(block_data));
	
	SQLSceneLoader loader(":memory:");

	scene.AddBlock(BlockModel{ original_block });

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
	ASSERT_TRUE(loaded_scene.has_value());
	auto loaded_block = loaded_scene.value().GetBlockById(block_id);
	ASSERT_TRUE(loaded_block);
	auto loaded_block_data = loaded_scene->GetFunctionalBlocksManager().GetDataForId(block_id);
	ASSERT_TRUE(loaded_block_data);

	EXPECT_EQ(loaded_block_data->block_class, class_name);
	
	auto&& loaded_properties = loaded_block_data->properties;
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

	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, model::BlockType::Functional, block_rect};
	std::string_view property_name = "Value";
	std::string_view property2_name = "Value2";
	std::string class_name{ "test2" };

	model::FunctionalBlockData block_data;
	block_data.block_class = class_name;
	block_data.properties.push_back(BlockProperty{ std::string{property_name}, BlockPropertyType::Boolean, true });
	block_data.properties.push_back(BlockProperty{ std::string{property2_name}, BlockPropertyType::Integer, static_cast<int64_t>(2) });
	scene.GetFunctionalBlocksManager().SetDataForId(block_id, std::move(block_data));

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(BlockModel{ original_block });

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
	ASSERT_TRUE(loaded_scene.has_value());
	auto loaded_block = loaded_scene.value().GetBlockById(block_id);
	ASSERT_TRUE(loaded_block);

	auto loaded_block_data = loaded_scene->GetFunctionalBlocksManager().GetDataForId(block_id);
	ASSERT_TRUE(loaded_block_data);

	EXPECT_EQ(loaded_block_data->block_class, class_name);

	auto&& loaded_properties = loaded_block_data->properties;
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

	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, BlockType::Functional, block_rect };
	std::string property_name = "Multiplier";
	std::string property_value = "Value1";	
	std::string styler_name{ "test1" };
	BlockStyleProperties properties{ {{property_name,property_value}} };
	original_block.SetStylerProperties(properties);
	original_block.SetStyler(styler_name);

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(BlockModel{ original_block });

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
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

	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, BlockType::Functional, block_rect };
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

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
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


TEST(testBlockLoader, testSaveLoadBlockSockets)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };


	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, BlockType::Functional, block_rect };
	auto socket_type = BlockSocketModel::SocketType::input;
	auto socket_id = SocketId{ 2 };
	Point socket_pos{ 5,5 };
	NetNodeId connected_node{ 4 };
	BlockSocketModel socket1{ socket_type, socket_id, socket_pos, connected_node };
	original_block.AddSocket(socket1);
	scene.AddBlock(BlockModel{ original_block });

	SQLSceneLoader loader(":memory:");

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 1);
	auto loaded_block_opt = loaded_scene.value().GetBlockById(block_id);

	ASSERT_TRUE(loaded_block_opt);
	auto&& loaded_block = loaded_block_opt->get();
	ASSERT_EQ(loaded_block.GetSockets().size(), 1);

	auto&& loaded_socket = loaded_block.GetSockets()[0];
	EXPECT_EQ(loaded_socket.GetId(), socket_id);
	EXPECT_EQ(loaded_socket.GetPosition(), socket_pos);
	EXPECT_EQ(loaded_socket.GetType(), socket_type);
	ASSERT_TRUE(loaded_socket.GetConnectedNetNode());
	ASSERT_EQ(*loaded_socket.GetConnectedNetNode(), connected_node);
}

TEST(testBlockLoader, testSaveLoadBlockSockets2)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };

	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, BlockType::Functional, block_rect };
	auto socket_type = BlockSocketModel::SocketType::output;
	auto socket_id = SocketId{ 3 };
	Point socket_pos{ 5,5 };
	BlockSocketModel socket1{ socket_type, socket_id, socket_pos };
	original_block.AddSocket(socket1);
	auto socket_type2 = BlockSocketModel::SocketType::input;
	auto socket_id2 = SocketId{ 0 };
	Point socket_pos2{ 6,6 };
	BlockSocketModel socket2{ socket_type2, socket_id2, socket_pos2 };
	original_block.AddSocket(socket2);
	scene.AddBlock(BlockModel{ original_block });

	SQLSceneLoader loader(":memory:");

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
	ASSERT_TRUE(loaded_scene.has_value());
	EXPECT_EQ(loaded_scene.value().GetBlocks().size(), 1);
	auto loaded_block_opt = loaded_scene.value().GetBlockById(block_id);

	ASSERT_TRUE(loaded_block_opt);
	auto&& loaded_block = loaded_block_opt->get();
	ASSERT_EQ(loaded_block.GetSockets().size(), 2);

	auto&& loaded_socket = loaded_block.GetSockets()[0];
	EXPECT_EQ(loaded_socket.GetId(), socket_id);
	EXPECT_EQ(loaded_socket.GetPosition(), socket_pos);
	EXPECT_EQ(loaded_socket.GetType(), socket_type);

	auto&& loaded_socket2 = loaded_block.GetSockets()[1];
	EXPECT_EQ(loaded_socket2.GetId(), socket_id2);
	EXPECT_EQ(loaded_socket2.GetPosition(), socket_pos2);
	EXPECT_EQ(loaded_socket2.GetType(), socket_type2);
}


TEST(testBlockLoader, testSaveLoadBlockSubSystem)
{
	NodeSceneModel scene;
	BlockId block_id{ 1 };

	SubSceneId subscene_id{ 1 };
	SubSceneId parent_subscene_id{ 0 };

	Rect block_rect{ 1,1,10,10 };
	BlockModel original_block{ block_id, model::BlockType::SubSystem, block_rect };
	std::string_view property_name = "Multiplier";
	std::string class_name{ "test1" };
	model::SubsystemBlockData block_data;
	std::string block_url = "FOO";
	SubSceneId subsystem_id{ 1 };
	block_data.URL = block_url;
	block_data.scene_id = subsystem_id;
	scene.GetSubsystemBlocksManager().SetDataForId(block_id, std::move(block_data));

	SQLSceneLoader loader(":memory:");

	scene.AddBlock(BlockModel{ original_block });

	auto result = loader.Save(scene, subscene_id, parent_subscene_id);

	ASSERT_TRUE(result);

	auto loaded_scene = loader.Load(subscene_id);
	ASSERT_TRUE(loaded_scene.has_value());
	auto loaded_block = loaded_scene.value().GetBlockById(block_id);
	ASSERT_TRUE(loaded_block);
	auto loaded_block_data = loaded_scene->GetSubsystemBlocksManager().GetDataForId(block_id);
	ASSERT_TRUE(loaded_block_data);

	EXPECT_EQ(loaded_block_data->URL, block_url);
	EXPECT_EQ(loaded_block_data->scene_id, subsystem_id);
}