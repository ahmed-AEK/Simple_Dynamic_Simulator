#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/IGraphicsScene.hpp"
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "NodeSDLStylers/DefaultBlockStyler.hpp"
#include <utility>

class GraphicsSceneMock: public node::IGraphicsScene {
public:
	MOCK_METHOD(node::SpaceScreenTransformer&, GetSpaceScreenTransformer, (), (const,override));
	MOCK_METHOD(void, InvalidateRect, (), (override));
	MOCK_METHOD(bool, IsObjectSelected, (const node::GraphicsObject&), (const, override));
};

using ::testing::ReturnRef;
using ::testing::Return;

TEST(testBlockObject, testCreate)
{
	GraphicsSceneMock mockScene;

	node::BlockObject block(&mockScene);

	auto ScreenRect = block.GetSpaceRect();
	EXPECT_EQ(ScreenRect.x, 100);
	EXPECT_EQ(ScreenRect.y, 100);
	EXPECT_EQ(ScreenRect.w, 100);
	EXPECT_EQ(ScreenRect.h, 100);

}

TEST(testBlockObject, testAddSocket)
{
	using namespace node;
	auto model = std::make_shared<node::model::BlockModel>(model::BlockId{0}, model::Rect{ 100,100,100,100 });
	model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0} });
	model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 0 } });
	auto styler = std::make_unique<node::DefaultBlockStyler>();
	node::BlockObject block{ nullptr, model::Rect{0,0,0,0}, std::move(styler), model->GetId() };
}


TEST(testNode, testConnectDisconnectSockets)
{

}



TEST(testNode, testLMBClick)
{
	
}