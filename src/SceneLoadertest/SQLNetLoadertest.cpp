//#pragma warning( push , 0)
#include "gtest/gtest.h"
//#pragma warning( pop ) 
#include "SceneLoader/SQLBlockLoader.hpp"
#include "SceneLoader/SQLSceneLoader.hpp"

using namespace node::loader;
using namespace node::model;

TEST(testNetLoader, testSaveLoadNetNode)
{
	NetNodeId node_id{ 1 };
	NetNodeModel node{ node_id, {10,10} };

	NodeSceneModel scene;
	scene.AddNetNode(std::move(node));

	SQLSceneLoader loader(":memory:");
	loader.Save(scene);

	auto new_scene = loader.Load();

	ASSERT_TRUE(new_scene);

	auto loaded_node_opt = new_scene->GetNetNodeById(node_id);
	ASSERT_TRUE(loaded_node_opt);
	ASSERT_EQ(new_scene->GetNetNodes().size(), 1);

	auto&& loaded_node = loaded_node_opt->get();
	EXPECT_EQ(loaded_node.GetId(), node_id);
	Point expected_pos{ 10,10 };
	EXPECT_EQ(loaded_node.GetPosition(), expected_pos);
	EXPECT_FALSE(loaded_node.GetSegmentAt(ConnectedSegmentSide::north));
	EXPECT_FALSE(loaded_node.GetSegmentAt(ConnectedSegmentSide::south));
	EXPECT_FALSE(loaded_node.GetSegmentAt(ConnectedSegmentSide::west));
	EXPECT_FALSE(loaded_node.GetSegmentAt(ConnectedSegmentSide::east));
}

TEST(testNetLoader, testSaveLoadNetNode2)
{
	NetNodeId node_id{ 3 };
	NetNodeModel node{ node_id, {20,20} };
	node.SetSegmentAt(ConnectedSegmentSide::north, NetSegmentId{ 1 });
	node.SetSegmentAt(ConnectedSegmentSide::south, NetSegmentId{ 2 });
	node.SetSegmentAt(ConnectedSegmentSide::west, NetSegmentId{ 3 });
	node.SetSegmentAt(ConnectedSegmentSide::east, NetSegmentId{ 4 });
	NodeSceneModel scene;
	scene.AddNetNode(std::move(node));

	SQLSceneLoader loader(":memory:");
	loader.Save(scene);

	auto new_scene = loader.Load();

	ASSERT_TRUE(new_scene);

	auto loaded_node_opt = new_scene->GetNetNodeById(node_id);
	ASSERT_TRUE(loaded_node_opt);
	ASSERT_EQ(new_scene->GetNetNodes().size(), 1);

	auto&& loaded_node = loaded_node_opt->get();
	EXPECT_EQ(loaded_node.GetId(), node_id);
	Point expected_pos{ 20,20 };
	EXPECT_EQ(loaded_node.GetPosition(), expected_pos);
	EXPECT_EQ(*loaded_node.GetSegmentAt(ConnectedSegmentSide::north), NetSegmentId{ 1 });
	EXPECT_EQ(*loaded_node.GetSegmentAt(ConnectedSegmentSide::south), NetSegmentId{ 2 });
	EXPECT_EQ(*loaded_node.GetSegmentAt(ConnectedSegmentSide::west), NetSegmentId{ 3 });
	EXPECT_EQ(*loaded_node.GetSegmentAt(ConnectedSegmentSide::east), NetSegmentId{ 4 });
}


TEST(testNetLoader, testSaveLoadNetSegment)
{
	NetSegmentId segment_id{ 1 };
	NetNodeId node1{ 2 };
	NetNodeId node2{ 3 };
	auto orientation{ NetSegmentOrientation::horizontal };
	NetSegmentModel segment{ segment_id, node1, node2, orientation};

	NodeSceneModel scene;
	scene.AddNetSegment(std::move(segment));

	SQLSceneLoader loader(":memory:");
	loader.Save(scene);

	auto new_scene = loader.Load();

	ASSERT_TRUE(new_scene);

	auto loaded_segment_opt = new_scene->GetNetSegmentById(segment_id);
	ASSERT_TRUE(loaded_segment_opt);
	ASSERT_EQ(new_scene->GetNetSegments().size(), 1);

	auto&& loaded_segment = loaded_segment_opt->get();
	EXPECT_EQ(loaded_segment.GetId(), segment_id);
	EXPECT_EQ(loaded_segment.m_firstNodeId, node1);
	EXPECT_EQ(loaded_segment.m_secondNodeId, node2);
	EXPECT_EQ(loaded_segment.m_orientation, orientation);
}

TEST(testNetLoader, testSaveLoadNetSegment2)
{
	NetSegmentId segment_id{ 3 };
	NetNodeId node1{ 1 };
	NetNodeId node2{ 4 };
	auto orientation{ NetSegmentOrientation::vertical };
	NetSegmentModel segment{ segment_id, node1, node2, orientation };

	NodeSceneModel scene;
	scene.AddNetSegment(std::move(segment));

	SQLSceneLoader loader(":memory:");
	loader.Save(scene);

	auto new_scene = loader.Load();

	ASSERT_TRUE(new_scene);

	auto loaded_segment_opt = new_scene->GetNetSegmentById(segment_id);
	ASSERT_TRUE(loaded_segment_opt);
	ASSERT_EQ(new_scene->GetNetSegments().size(), 1);

	auto&& loaded_segment = loaded_segment_opt->get();
	EXPECT_EQ(loaded_segment.GetId(), segment_id);
	EXPECT_EQ(loaded_segment.m_firstNodeId, node1);
	EXPECT_EQ(loaded_segment.m_secondNodeId, node2);
	EXPECT_EQ(loaded_segment.m_orientation, orientation);
}

TEST(testNetLoader, testSaveLoadSocketConnection)
{
	NetNodeId node_id{ 1 };
	SocketId socket_id{ 2 };
	BlockId block_id{ 3 };

	SocketNodeConnection connecion{ {socket_id, block_id}, node_id };

	NodeSceneModel scene;
	scene.AddSocketNodeConnection(std::move(connecion));

	SQLSceneLoader loader(":memory:");
	loader.Save(scene);

	auto new_scene = loader.Load();

	ASSERT_TRUE(new_scene);

	auto loaded_connection_opt = new_scene->GetSocketConnectionForNode(node_id);
	ASSERT_TRUE(loaded_connection_opt);
	ASSERT_EQ(new_scene->GetSocketConnections().size(), 1);

	auto&& loaded_connection = loaded_connection_opt->get();
	EXPECT_EQ(loaded_connection.NodeId, node_id);
	EXPECT_EQ(loaded_connection.socketId.socket_id, socket_id);
	EXPECT_EQ(loaded_connection.socketId.block_id, block_id);
}


TEST(testNetLoader, testSaveLoadSocketConnection2)
{

	NodeSceneModel scene;

	{
		NetNodeId node_id{ 1 };
		SocketId socket_id{ 2 };
		BlockId block_id{ 3 };

		SocketNodeConnection connecion{ {socket_id, block_id}, node_id };
		scene.AddSocketNodeConnection(std::move(connecion));
	}

	NetNodeId node_id{ 4 };
	SocketId socket_id{ 5 };
	BlockId block_id{ 6 };

	SocketNodeConnection connecion{ {socket_id, block_id}, node_id };

	scene.AddSocketNodeConnection(std::move(connecion));

	SQLSceneLoader loader(":memory:");
	loader.Save(scene);

	auto new_scene = loader.Load();

	ASSERT_TRUE(new_scene);

	auto loaded_connection_opt = new_scene->GetSocketConnectionForNode(node_id);
	ASSERT_TRUE(loaded_connection_opt);
	ASSERT_EQ(new_scene->GetSocketConnections().size(), 2);

	auto&& loaded_connection = loaded_connection_opt->get();
	EXPECT_EQ(loaded_connection.NodeId, node_id);
	EXPECT_EQ(loaded_connection.socketId.socket_id, socket_id);
	EXPECT_EQ(loaded_connection.socketId.block_id, block_id);
}

