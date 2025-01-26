#pragma once

#include "NodeModels/NetModel.hpp"

namespace node
{

struct NetSolutionEndDescription
{
	model::Point point;
	std::array<bool, 4> allowed_sides;

	bool IsSideAllowed(model::ConnectedSegmentSide side) const
	{
		return allowed_sides[static_cast<size_t>(side)];
	}
	void SetSideAllowed(model::ConnectedSegmentSide side, bool value = true)
	{
		allowed_sides[static_cast<size_t>(side)] = value;
	}
};

class NetsSolver
{
public:
	struct NetSolution
	{
		std::vector<model::NetNodeModel> nodes;
		std::vector<model::NetSegmentModel> segments;
		std::optional<model::NetNodeId> start;
		std::optional<model::NetNodeId> end;
	};

	void SetStartDescription(const NetSolutionEndDescription& d)
	{
		m_start_node = d;
	}
	void SetEndDescription(const NetSolutionEndDescription& d)
	{
		m_end_node = d;
	}
	void SetExtensionDistance(model::node_int distance)
	{
		m_extension_distance = distance;
	}
	model::node_int GetExtensionDistance() const { return m_extension_distance; }
	NetSolution Solve();
	void Reset();
private:

	struct InnerResultRemap
	{
		size_t start;
		size_t end;
	};

	InnerResultRemap AddInnerResult(const NetSolution& inner_result, NetSolution& result);

	NetSolution SolverHorz();
	NetSolution SolveVert();
	NetSolution SolveZ();
	NetSolution SolveN();
	NetSolution SolveL();
	NetSolution SolveHFlipL();
	NetSolution ExtendOneThenSolve(model::ConnectedSegmentSide side);
	NetSolution ExtendTwoThenSolve(model::NetSegmentOrientation orientation);
	NetsSolver CreateSimilarSolver() const;
	model::NetNodeId GetNewNodeId();
	model::NetSegmentId GetNewSegmentId();

	std::optional<NetSolutionEndDescription> m_start_node;
	std::optional<NetSolutionEndDescription> m_end_node;
	model::id_int m_new_node_id = 0;
	model::id_int m_new_segment_id = 0;
	model::node_int m_extension_distance = 30;
};


}