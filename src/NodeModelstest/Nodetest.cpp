//#pragma warning( push , 0)
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
//#pragma warning( pop ) 
#include "NodeModels/NodeModel.hpp"

using namespace node::model;

class NetObserverMock : public Observer<NodeEventArg>
{
public:
	MOCK_METHOD(void, OnEvent, (NodeEventArg&), (override));

};

bool operator==(const NodeEventArg& op1, const NodeEventArg& op2)
{
	return &(op1.object) == &(op2.object) && op1.event == op2.event;
}

MATCHER_P(ArgEq, type, "") {
	(void)(result_listener);
	return arg == type;
}

TEST(testNode, testCreate)
{
	NodeModel node1{ 1 , {10,10,10,10} };
}

TEST(testNode, testAddSocket)
{
	NodeModel node1{ 1 , {10,10,10,10} };
	NodeSocketModel socket1{ NodeSocketModel::SocketType::input, {2, node1.GetId()}, {0,5} };
	
	NetObserverMock observer;
	node1.Attach(&observer);
	NodeEventArg event1{ node1, NodeEvent::SocketsChanged };

	EXPECT_CALL(observer, OnEvent( ArgEq( event1 ) ) ).Times(1);

	node1.AddSocket(socket1);

	ASSERT_EQ(node1.GetSockets().size(), 1);
	EXPECT_TRUE(node1.GetSocketById(socket1.GetId().m_Id).has_value());
	EXPECT_FALSE(node1.GetSocketById(1).has_value());

}

TEST(testNode, testChangePosition)
{
	Point old_position = { 10,10 };

	NodeModel node1{ 1 , {old_position,10,10} };

	NetObserverMock observer;
	node1.Attach(&observer);
	NodeEventArg event1{ node1, NodeEvent::PositionChanged};

	EXPECT_CALL(observer, OnEvent(ArgEq(event1))).Times(1);

	EXPECT_EQ(node1.GetPosition(), old_position);
	Point new_pos = { 0,0 };
	node1.SetPosition(new_pos);

	EXPECT_EQ(node1.GetPosition(), new_pos);

}