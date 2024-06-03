#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GraphicsScene/Node.hpp"
#include "GraphicsScene/IGraphicsScene.hpp"
#include "GraphicsScene/SpaceScreenTransformer.hpp"
#include "GraphicsScene/IGraphicsSceneController.hpp"
#include "GraphicsScene/NodeSocket.hpp"
#include "GraphicsScene/NetObject.hpp"
#include <utility>

class GraphicsSceneMock: public node::IGraphicsScene {
public:
	MOCK_METHOD(node::SpaceScreenTransformer&, GetSpaceScreenTransformer, (), (const,override));
	MOCK_METHOD(void, InvalidateRect, (), (override));
	MOCK_METHOD(bool, IsObjectSelected, (const node::GraphicsObject&), (const, override));
	MOCK_METHOD(node::IGraphicsSceneController*, GetController, (), (const, override));

};

class GraphicsSceneControllerMock : public node::IGraphicsSceneController
{
public:
	MOCK_METHOD(MI::ClickEvent, OnNodeLMBDown, (const SDL_Point&, node::Node&), (override));
	MOCK_METHOD(MI::ClickEvent, OnSocketLMBDown, (const SDL_Point&, node::NodeSocket&), (override));
	MOCK_METHOD(MI::ClickEvent, OnSegmentLMBDown, (const SDL_Point&, node::NetSegment&), (override));
	MOCK_METHOD(MI::ClickEvent, OnNetNodeLMBDown, (const SDL_Point&, node::NetNode&), (override));

};

using ::testing::ReturnRef;
using ::testing::Return;

TEST(testNode, testCreate)
{
	auto transformer = node::SpaceScreenTransformer{ { 1000,500,800,600 }, { 100,100,100,100 } };
	GraphicsSceneMock mockScene;

	EXPECT_CALL(mockScene, GetSpaceScreenTransformer())
		.Times(1)
		.WillOnce(ReturnRef(transformer));

	node::Node node({ 100,100,100,100 }, &mockScene);

	auto ScreenRect = node.GetRectImpl();
	EXPECT_EQ(ScreenRect.x, 1000);
	EXPECT_EQ(ScreenRect.y, 500);
	EXPECT_EQ(ScreenRect.w, 800);
	EXPECT_EQ(ScreenRect.h, 600);

}

TEST(testNode, testAddSocket)
{
	node::Node node({ 100,100,100,100 }, nullptr);

	auto initial_sockets = node.GetSockets();

	node.AddInputSocket(1);

	auto sockets_after_1_input = node.GetSockets();

	node.AddOutputSocket(2);

	auto sockets_after_1_output = node.GetSockets();

	EXPECT_EQ(initial_sockets.size(), 0);
	ASSERT_EQ(sockets_after_1_input.size(), 1);
	EXPECT_EQ((sockets_after_1_input[0]->GetSocketType()), node::SocketType::input);
	ASSERT_EQ(sockets_after_1_output.size(), 2);
	EXPECT_EQ((sockets_after_1_output[1]->GetSocketType()), node::SocketType::output);
}


TEST(testNode, testConnectDisconnectSockets)
{
	node::Node node({ 100,100,100,100 }, nullptr);

	node.AddInputSocket(1);
	node.AddOutputSocket(2);

	node::NetNode node1{ SDL_Point{0,0}, nullptr };
	node::NetNode node2{ SDL_Point{0,0}, nullptr };
	auto sockets_after_2_sockets = node.GetSockets();
	sockets_after_2_sockets[0]->SetConnectedNode(&node1);
	sockets_after_2_sockets[1]->SetConnectedNode(&node2);

	auto connected_socket1 = node1.GetConnectedSocket();
	auto connected_socket2 = node2.GetConnectedSocket();
	auto connected_node1 = sockets_after_2_sockets[0]->GetConnectedNode();
	auto connected_node2 = sockets_after_2_sockets[1]->GetConnectedNode();

	node.DisconnectSockets();

	ASSERT_EQ(sockets_after_2_sockets.size(), 2);
	EXPECT_EQ(connected_socket1, sockets_after_2_sockets[0]);
	EXPECT_EQ(connected_socket2, sockets_after_2_sockets[1]);
	EXPECT_EQ(connected_node1, &node1);
	EXPECT_EQ(connected_node2, &node2);
	EXPECT_EQ(node1.GetConnectedSocket(), nullptr);
	EXPECT_EQ(node2.GetConnectedSocket(), nullptr);
	EXPECT_EQ(sockets_after_2_sockets[0]->GetConnectedNode(), nullptr);
	EXPECT_EQ(sockets_after_2_sockets[1]->GetConnectedNode(), nullptr);
}



TEST(testNode, testLMBClick)
{
	auto transformer = node::SpaceScreenTransformer{ { 1000,500,800,600 }, { 100,100,100,100 } };
	GraphicsSceneMock mockScene;
	GraphicsSceneControllerMock mockController;

	EXPECT_CALL(mockScene, GetSpaceScreenTransformer())
		.WillRepeatedly(ReturnRef(transformer));


	node::Node node({ 100,100,100,100 }, &mockScene);

	EXPECT_CALL(mockScene, GetController())
		.Times(1)
		.WillOnce(Return(&mockController));

	EXPECT_CALL(mockController, OnNodeLMBDown)
		.Times(1)
		.WillOnce(Return(MI::ClickEvent::CLICKED));
	
	auto result = node.LMBDown({ 150,150 });

	EXPECT_EQ(result, MI::ClickEvent::CLICKED);
}