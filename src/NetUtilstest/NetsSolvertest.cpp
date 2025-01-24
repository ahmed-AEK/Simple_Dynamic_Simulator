//#pragma warning( push , 0)
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
//#pragma warning( pop ) 
#include "NetUtils/NetsSolver.hpp"
#include "NetUtils/Utils.hpp"

using namespace node;
using namespace node::model;
using ::testing::UnorderedElementsAreArray;
TEST(testNetsSolver, testCreate)
{
	NetsSolver solver;
	(void)solver;
}
TEST(testNetsSolver, testHorzStraightLines)
{
	NetsSolver solver;
	NetsSolver::SolutionEndDescription node1{ model::Point{0,0}, {true,true,true,true} };
	NetsSolver::SolutionEndDescription node2{ model::Point{1,0}, {true,true,true,true} };
	solver.SetStartDescription(node1);
	solver.SetEndDescription(node2);
	auto solution1 = solver.Solve();
	solver.Reset();
	solver.SetStartDescription(node2);
	solver.SetEndDescription(node1);
	auto solution2 = solver.Solve();

	{
		const auto& solution = solution1;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node1.point, NetSegmentOrientation::horizontal},
				{node2.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{ 
				solution.segments[i].m_firstNodeId, 
				solution.segments[i].m_secondNodeId};
			std::vector<model::NetNodeId> expected_arr{ 
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId};
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}

	{
		const auto& solution = solution2;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node2.point, NetSegmentOrientation::horizontal},
				{node1.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}

	}
}

TEST(testNetsSolver, testVertStraightLines)
{
	NetsSolver solver;
	NetsSolver::SolutionEndDescription node1{ model::Point{0,0}, {true,true,true,true} };
	NetsSolver::SolutionEndDescription node2{ model::Point{0,1}, {true,true,true,true} };
	solver.SetStartDescription(node1);
	solver.SetEndDescription(node2);
	auto solution1 = solver.Solve();
	solver.Reset();
	solver.SetStartDescription(node2);
	solver.SetEndDescription(node1);
	auto solution2 = solver.Solve();

	{
		const auto& solution = solution1;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node1.point, NetSegmentOrientation::vertical},
				{node2.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}

	{
		const auto& solution = solution2;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node2.point, NetSegmentOrientation::vertical},
				{node1.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}
}

TEST(testNetsSolver, testLShape)
{
	NetsSolver solver;
	NetsSolver::SolutionEndDescription node1{ model::Point{0,0}, {false,false,true,false} };
	NetsSolver::SolutionEndDescription node2{ model::Point{1,1}, {false,false,false,true} };
	solver.SetStartDescription(node1);
	solver.SetEndDescription(node2);
	auto solution1 = solver.Solve();
	solver.Reset();
	solver.SetStartDescription(node2);
	solver.SetEndDescription(node1);
	auto solution2 = solver.Solve();

	{
		const auto& solution = solution1;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node1.point, NetSegmentOrientation::vertical},
				{node1.point.x, node2.point.y, NetSegmentOrientation::horizontal},
				{node2.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}

	{
		const auto& solution = solution2;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node2.point, NetSegmentOrientation::horizontal},
				{node1.point.x, node2.point.y, NetSegmentOrientation::vertical},
				{node1.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}
}


TEST(testNetsSolver, testVerticalFlippedLShape)
{
	NetsSolver solver;
	NetsSolver::SolutionEndDescription node1{ model::Point{0,1}, {true,false,false,false} };
	NetsSolver::SolutionEndDescription node2{ model::Point{1,0}, {false,false,false,true} };
	solver.SetStartDescription(node1);
	solver.SetEndDescription(node2);
	auto solution1 = solver.Solve();
	solver.Reset();
	solver.SetStartDescription(node2);
	solver.SetEndDescription(node1);
	auto solution2 = solver.Solve();

	{
		const auto& solution = solution1;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node1.point, NetSegmentOrientation::vertical},
				{node1.point.x, node2.point.y, NetSegmentOrientation::horizontal},
				{node2.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}

	{
		const auto& solution = solution2;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node2.point, NetSegmentOrientation::horizontal},
				{node1.point.x, node2.point.y, NetSegmentOrientation::vertical},
				{node1.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}
}

TEST(testNetsSolver, testHorizontalFlippedLShape)
{
	NetsSolver solver;
	NetsSolver::SolutionEndDescription node1{ model::Point{0,1}, {false,true,false,false} };
	NetsSolver::SolutionEndDescription node2{ model::Point{1,0}, {false,false,true,false} };
	solver.SetStartDescription(node1);
	solver.SetEndDescription(node2);
	auto solution1 = solver.Solve();
	solver.Reset();
	solver.SetStartDescription(node2);
	solver.SetEndDescription(node1);
	auto solution2 = solver.Solve();

	{
		const auto& solution = solution1;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node1.point, NetSegmentOrientation::horizontal},
				{node2.point.x, node1.point.y, NetSegmentOrientation::vertical},
				{node2.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}

	{
		const auto& solution = solution2;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node2.point, NetSegmentOrientation::vertical},
				{node2.point.x, node1.point.y, NetSegmentOrientation::horizontal},
				{node1.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}
}

TEST(testNetsSolver, testZshape)
{
	NetsSolver solver;
	NetsSolver::SolutionEndDescription node1{ model::Point{0,0}, {false,true,false,false} };
	NetsSolver::SolutionEndDescription node2{ model::Point{10,20}, {false,false,false,true} };
	solver.SetStartDescription(node1);
	solver.SetEndDescription(node2);
	auto solution1 = solver.Solve();
	solver.Reset();
	solver.SetStartDescription(node2);
	solver.SetEndDescription(node1);
	auto solution2 = solver.Solve();

	{
		const auto& solution = solution1;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node1.point, NetSegmentOrientation::horizontal},
				{{(node1.point.x + node2.point.x) / 2, node1.point.y}, NetSegmentOrientation::vertical},
				{{(node1.point.x + node2.point.x) / 2, node2.point.y}, NetSegmentOrientation::horizontal},
				{node2.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}

	{
		const auto& solution = solution2;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node2.point, NetSegmentOrientation::horizontal},
				{{(node2.point.x + node1.point.x) / 2, node2.point.y}, NetSegmentOrientation::vertical},
				{{(node2.point.x + node1.point.x) / 2, node1.point.y}, NetSegmentOrientation::horizontal},
				{node1.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}
}

TEST(testNetsSolver, testHorizontalFlippedZshape)
{
	NetsSolver solver;
	NetsSolver::SolutionEndDescription node1{ model::Point{10,0}, {false,false,false,true} };
	NetsSolver::SolutionEndDescription node2{ model::Point{0,20}, {false,true,false,false} };
	solver.SetStartDescription(node1);
	solver.SetEndDescription(node2);
	auto solution1 = solver.Solve();
	solver.Reset();
	solver.SetStartDescription(node2);
	solver.SetEndDescription(node1);
	auto solution2 = solver.Solve();

	{
		const auto& solution = solution1;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node1.point, NetSegmentOrientation::horizontal},
				{{(node1.point.x + node2.point.x) / 2, node1.point.y}, NetSegmentOrientation::vertical},
				{{(node1.point.x + node2.point.x) / 2, node2.point.y}, NetSegmentOrientation::horizontal},
				{node2.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}

	{
		const auto& solution = solution2;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node2.point, NetSegmentOrientation::horizontal},
				{{(node2.point.x + node1.point.x) / 2, node2.point.y}, NetSegmentOrientation::vertical},
				{{(node2.point.x + node1.point.x) / 2, node1.point.y}, NetSegmentOrientation::horizontal},
				{node1.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}
}


TEST(testNetsSolver, testUShape)
{
	NetsSolver solver;
	node_int extension_distance = 30;
	solver.SetExtensionDistance(extension_distance);
	NetsSolver::SolutionEndDescription node1{ model::Point{0,0}, {false,false,true,false} };
	NetsSolver::SolutionEndDescription node2{ model::Point{20,0}, {false,false,true,false} };
	solver.SetStartDescription(node1);
	solver.SetEndDescription(node2);
	auto solution1 = solver.Solve();
	solver.Reset();
	solver.SetStartDescription(node2);
	solver.SetEndDescription(node1);
	auto solution2 = solver.Solve();

	{
		const auto& solution = solution1;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node1.point, NetSegmentOrientation::vertical},
				{{node1.point.x, node1.point.y + extension_distance}, NetSegmentOrientation::horizontal},
				{{node2.point.x, node1.point.y  + extension_distance}, NetSegmentOrientation::vertical},
				{node2.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}

	{
		const auto& solution = solution2;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node2.point, NetSegmentOrientation::vertical},
				{{node2.point.x, node2.point.y + extension_distance}, NetSegmentOrientation::horizontal},
				{{node1.point.x, node2.point.y + extension_distance}, NetSegmentOrientation::vertical},
				{node1.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}
}

TEST(testNetsSolver, testDShape)
{
	NetsSolver solver;
	node_int extension_distance = 30;
	solver.SetExtensionDistance(extension_distance);
	NetsSolver::SolutionEndDescription node1{ model::Point{0,0}, {false,true,false,false} };
	NetsSolver::SolutionEndDescription node2{ model::Point{0,20}, {false,true,false,false} };
	solver.SetStartDescription(node1);
	solver.SetEndDescription(node2);
	auto solution1 = solver.Solve();
	solver.Reset();
	solver.SetStartDescription(node2);
	solver.SetEndDescription(node1);
	auto solution2 = solver.Solve();

	{
		const auto& solution = solution1;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node1.point, NetSegmentOrientation::horizontal},
				{{node1.point.x + extension_distance, node1.point.y}, NetSegmentOrientation::vertical},
				{{node1.point.x + extension_distance, node2.point.y}, NetSegmentOrientation::horizontal},
				{node2.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}

	{
		const auto& solution = solution2;

		auto expected_result = NetUtils::CreateStringNet(
			NetUtils::StringNetDescription{
				{node2.point, NetSegmentOrientation::horizontal},
				{{node2.point.x + extension_distance, node2.point.y}, NetSegmentOrientation::vertical},
				{{node2.point.x + extension_distance, node1.point.y}, NetSegmentOrientation::horizontal},
				{node1.point, {}},
			}
			);

		ASSERT_EQ(solution.nodes.size(), expected_result.nodes.size());
		ASSERT_EQ(solution.segments.size(), expected_result.segments.size());

		ASSERT_EQ(solution.nodes[0].GetId(), solution.start);
		ASSERT_EQ(solution.nodes.back().GetId(), solution.end);

		for (size_t i = 0; i < solution.nodes.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.nodes[i].GetId(), NetNodeId{ static_cast<id_int>(i) });
			for (int j = 0; j < 4; j++)
			{
				SCOPED_TRACE("j is : " + std::to_string(j));
				EXPECT_EQ(solution.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)),
					expected_result.nodes[i].GetSegmentAt(static_cast<ConnectedSegmentSide>(j)));
			}
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			ASSERT_EQ(solution.segments[i].GetId(), NetSegmentId{ static_cast<id_int>(i) });
		}
		for (size_t i = 0; i < solution.segments.size(); i++)
		{
			SCOPED_TRACE("i is : " + std::to_string(i));
			std::vector<model::NetNodeId> segment_arr{
				solution.segments[i].m_firstNodeId,
				solution.segments[i].m_secondNodeId };
			std::vector<model::NetNodeId> expected_arr{
				expected_result.segments[i].m_firstNodeId ,
				expected_result.segments[i].m_secondNodeId };
			EXPECT_THAT(segment_arr, UnorderedElementsAreArray(expected_arr));
			EXPECT_EQ(solution.segments[i].m_orientation, expected_result.segments[i].m_orientation);
		}
	}
}
