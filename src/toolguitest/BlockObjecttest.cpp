


using ::testing::ReturnRef;
using ::testing::Return;

TEST(testBlockObject, testCreate)
{
	node::BlockObject block{};

	auto ScreenRect = block.GetSceneRect();
	EXPECT_EQ(ScreenRect.x, 0);
	EXPECT_EQ(ScreenRect.y, 0);
	EXPECT_EQ(ScreenRect.w, 100);
	EXPECT_EQ(ScreenRect.h, 100);
}

TEST(testBlockObject, testAddSocket)
{
	using namespace node;
	auto model = std::make_shared<node::model::BlockModel>(model::BlockId{0}, model::BlockType::Functional, model::Rect{ 100,100,100,100 });
	model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::input, model::SocketId{ 0} });
	model->AddSocket(model::BlockSocketModel{ model::BlockSocketModel::SocketType::output, model::SocketId{ 0 } });
	auto styler = std::make_unique<node::DefaultBlockStyler>();
	node::BlockObject block{model::ObjectSize{ 0,0 }, std::move(styler), model->GetId() };
}


TEST(testNode, testConnectDisconnectSockets)
{

}



TEST(testNode, testLMBClick)
{
	
}