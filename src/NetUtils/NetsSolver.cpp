#include "NetsSolver.hpp"
#include "toolgui/NodeMacros.h"
#include <cassert>
#include "NetUtils/Utils.hpp"

static void CopyConnectivity(std::span<const node::model::NetNodeModel> input_nodes,
    std::span<const node::model::NetSegmentModel> input_segments,
    std::span<node::model::NetNodeModel> output_nodes,
    std::span<node::model::NetSegmentModel> output_segments
)
{
    using namespace node;
    using namespace node::model;
    assert(input_segments.size() == output_segments.size());
    assert(input_nodes.size() == output_nodes.size());

    for (size_t i = 0; i < input_segments.size(); i++)
    {
        auto& input_segment = input_segments[i];
        output_segments[i].m_orientation = input_segment.m_orientation;
        NetUtils::connectSegementAndNodes(output_segments[i],
            output_nodes[input_segment.m_firstNodeId.value], output_nodes[input_segment.m_secondNodeId.value]);
    }
}
node::NetsSolver::NetSolution node::NetsSolver::Solve()
{
    const NetSolutionEndDescription& start_node = *m_start_node;
    const NetSolutionEndDescription& end_node = *m_end_node;
    using namespace model;
    if (start_node.point.y == end_node.point.y)
    {
        // try a horizontal line
        bool start_is_left = start_node.point.x < end_node.point.x;
        auto& left_desc =  start_is_left ? start_node : end_node;
        auto& right_desc = start_is_left ? end_node : start_node;

        if (left_desc.IsSideAllowed(ConnectedSegmentSide::east) &&
            right_desc.IsSideAllowed(ConnectedSegmentSide::west))
        {
            return SolverHorz();
        }

        // try a U
        if (left_desc.IsSideAllowed(ConnectedSegmentSide::south) && 
            right_desc.IsSideAllowed(ConnectedSegmentSide::south))
        {
            return ExtendOneThenSolve(ConnectedSegmentSide::south);
        }

        // try two branches then a U
        if (left_desc.IsSideAllowed(ConnectedSegmentSide::west) &&
            right_desc.IsSideAllowed(ConnectedSegmentSide::east))
        {
            return ExtendTwoThenSolve(NetSegmentOrientation::horizontal);
        }

        assert(false); // unimplemented
        
    }
    if (start_node.point.x == end_node.point.x)
    {
        // try a vertical line
        bool start_is_top = start_node.point.y < end_node.point.y;
        auto& top_node = start_is_top ? start_node : end_node;
        auto& bot_node = start_is_top ? end_node : start_node;
        if (top_node.IsSideAllowed(ConnectedSegmentSide::south) &&
            bot_node.IsSideAllowed(ConnectedSegmentSide::north))
        {
            return SolveVert();
        }

        // try a D
        if (top_node.IsSideAllowed(ConnectedSegmentSide::east) &&
            bot_node.IsSideAllowed(ConnectedSegmentSide::east))
        {
            return ExtendOneThenSolve(ConnectedSegmentSide::east);
        }

        // try 2 branches then a D
        if (top_node.IsSideAllowed(ConnectedSegmentSide::north) &&
            bot_node.IsSideAllowed(ConnectedSegmentSide::south))
        {
            return ExtendTwoThenSolve(NetSegmentOrientation::vertical);
        }

        assert(false); // unimplemented
    }
    
    // at this point, they should be at different X and Y
    bool start_is_left = start_node.point.x < end_node.point.x;
    bool start_is_top = start_node.point.y < end_node.point.y;
    auto& left_desc = start_is_left ? start_node : end_node;
    auto& right_desc = start_is_left ? end_node : start_node;
    auto& top_desc = start_is_top ? start_node : end_node;
    auto& bot_desc = start_is_top ? end_node : start_node;

    // L
    if (&top_desc == &left_desc &&
        top_desc.IsSideAllowed(ConnectedSegmentSide::south) &&
        bot_desc.IsSideAllowed(ConnectedSegmentSide::west))
    {
        return SolveL();
    }

    // L VFlip
    if (&bot_desc == &left_desc &&
        bot_desc.IsSideAllowed(ConnectedSegmentSide::north) &&
        right_desc.IsSideAllowed(ConnectedSegmentSide::west))
    {
        return SolveL();
    }

    // L HFlip
    if (&bot_desc == &left_desc &&
        bot_desc.IsSideAllowed(ConnectedSegmentSide::east) &&
        right_desc.IsSideAllowed(ConnectedSegmentSide::south))
    {
        return SolveHFlipL();
    }

    // L HFlip and VFlip
    if (&top_desc == &left_desc &&
        top_desc.IsSideAllowed(ConnectedSegmentSide::east) &&
        bot_desc.IsSideAllowed(ConnectedSegmentSide::north))
    {
        return SolveHFlipL();
    }

    if (left_desc.IsSideAllowed(ConnectedSegmentSide::east) &&
        right_desc.IsSideAllowed(ConnectedSegmentSide::west))
    {
        return SolveZ();
    }
    if (top_desc.IsSideAllowed(ConnectedSegmentSide::south) &&
        bot_desc.IsSideAllowed(ConnectedSegmentSide::north))
    {
        return SolveN();
    }

    if (left_desc.IsSideAllowed(ConnectedSegmentSide::south) && right_desc.IsSideAllowed(ConnectedSegmentSide::south))
    {
        return ExtendOneThenSolve(ConnectedSegmentSide::south);
    }
    
    if (top_desc.IsSideAllowed(ConnectedSegmentSide::east) && bot_desc.IsSideAllowed(ConnectedSegmentSide::east))
    {
        return ExtendOneThenSolve(ConnectedSegmentSide::east);
    }
    if (left_desc.IsSideAllowed(ConnectedSegmentSide::west) &&
        right_desc.IsSideAllowed(ConnectedSegmentSide::east))
    {
        return ExtendTwoThenSolve(NetSegmentOrientation::horizontal);
    }
    assert(false); // shouldn't reach this point!
    return NetSolution{};
}

void node::NetsSolver::Reset()
{
    m_new_node_id = 0;
    m_new_segment_id = 0;
}

node::NetsSolver::InnerResultRemap node::NetsSolver::AddInnerResult(const NetSolution& inner_result, NetSolution& result)
{
    using namespace model;

    size_t initial_nodes = result.nodes.size();
    size_t initial_segments = result.segments.size();
    for (size_t i = 0; i < inner_result.nodes.size(); i++)
    {
        result.nodes.push_back(NetNodeModel{ GetNewNodeId(), inner_result.nodes[i].GetPosition()});
    }
    for (const auto& segment : inner_result.segments)
    {
        result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
            NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation{segment.m_orientation} });
    }

    CopyConnectivity(inner_result.nodes, inner_result.segments,
        std::span{ result.nodes }.subspan(initial_nodes), std::span{ result.segments }.subspan(initial_segments));

    return {static_cast<size_t>(inner_result.start->value) + initial_nodes , static_cast<size_t>(inner_result.end->value) + initial_nodes };
}

node::NetsSolver::NetSolution node::NetsSolver::SolverHorz()
{
    using namespace node;
    using namespace node::model;

    NetSolution result;
    result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_start_node->point });
    result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_end_node->point });
    result.start = result.nodes[0].GetId();
    result.end = result.nodes[1].GetId();

    result.segments.push_back(NetSegmentModel{ GetNewSegmentId(), result.nodes[0].GetId(), result.nodes[1].GetId(), NetSegmentOrientation::horizontal });
    NetUtils::connectSegementAndNodes(result.segments[0], result.nodes[0], result.nodes[1]);
    return result;
}

node::NetsSolver::NetSolution node::NetsSolver::SolveVert()
{
    using namespace node;
    using namespace node::model;

    NetSolution result;
    result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_start_node->point });
    result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_end_node->point });
    result.start = result.nodes[0].GetId();
    result.end = result.nodes[1].GetId();

    result.segments.push_back(NetSegmentModel{ GetNewSegmentId(), result.nodes[0].GetId(), result.nodes[1].GetId(), NetSegmentOrientation::vertical });
    NetUtils::connectSegementAndNodes(result.segments[0], result.nodes[0], result.nodes[1]);
    return result;
}

node::NetsSolver::NetSolution node::NetsSolver::SolveZ()
{
    using namespace node;
    using namespace node::model;

    NetSolution result;

    // at this point, they should be at different X and Y
    bool start_is_left = m_start_node->point.x <= m_end_node->point.x;

    // add start node
    result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_start_node->point });
    result.start = result.nodes[0].GetId();

    // add segment between start and inner solve
    result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
        NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::horizontal });

    // can do Z connection
    model::Point center{ m_start_node->point + m_end_node->point };
    center.x /= 2;
    center.y /= 2;

    NetSolutionEndDescription inner_start{ {center.x, m_start_node->point.y}, {true, true, true, true} };
    NetSolutionEndDescription inner_end{ {center.x, m_end_node->point.y}, {true, true, true, true} };
    if (start_is_left)
    {
        inner_start.SetSideAllowed(ConnectedSegmentSide::west, false);
        inner_end.SetSideAllowed(ConnectedSegmentSide::east, false);
    }
    else
    {
        inner_start.SetSideAllowed(ConnectedSegmentSide::east, false);
        inner_end.SetSideAllowed(ConnectedSegmentSide::west, false);
    }

    auto inner_solver{ CreateSimilarSolver() };
    inner_solver.SetStartDescription(inner_start);
    inner_solver.SetEndDescription(inner_end);
    auto inner_result = inner_solver.Solve();

    auto inner_remap = AddInnerResult(inner_result, result);

    // add end node
    result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_end_node->point });
    result.end = result.nodes.back().GetId();

    // add segment between end and inner solve
    result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
    NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::horizontal });

    NetUtils::connectSegementAndNodes(result.segments[0], result.nodes[0], result.nodes[1]);
    NetUtils::connectSegementAndNodes(result.segments.back(), result.nodes[inner_remap.end], result.nodes.back());
    
    return result;
}

node::NetsSolver::NetSolution node::NetsSolver::SolveN()
{
    using namespace node;
    using namespace node::model;

    NetSolution result;

    // at this point, they should be at different X and Y
    bool start_is_top = m_start_node->point.y <= m_end_node->point.y;

    // add start node
    result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_start_node->point });
    result.start = result.nodes[0].GetId();

    // add segment between start and inner solve
    result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
        NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::vertical });

    // can do Z connection
    model::Point center{ m_start_node->point + m_end_node->point };
    center.x /= 2;
    center.y /= 2;

    NetSolutionEndDescription inner_start{ {m_start_node->point.x, center.y}, {true, true, true, true} };
    NetSolutionEndDescription inner_end{ {m_end_node->point.x, center.y}, {true, true, true, true} };
    if (start_is_top)
    {
        inner_start.SetSideAllowed(ConnectedSegmentSide::north, false);
        inner_end.SetSideAllowed(ConnectedSegmentSide::south, false);
    }
    else
    {
        inner_start.SetSideAllowed(ConnectedSegmentSide::south, false);
        inner_end.SetSideAllowed(ConnectedSegmentSide::north, false);
    }

    auto inner_solver{ CreateSimilarSolver() };
    inner_solver.SetStartDescription(inner_start);
    inner_solver.SetEndDescription(inner_end);
    auto inner_result = inner_solver.Solve();

    auto inner_remap = AddInnerResult(inner_result, result);

    // add end node
    result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_end_node->point });
    result.end = result.nodes.back().GetId();

    // add segment between end and inner solve
    result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
    NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::vertical });

    NetUtils::connectSegementAndNodes(result.segments[0], result.nodes[0], result.nodes[1]);
    NetUtils::connectSegementAndNodes(result.segments.back(), result.nodes[inner_remap.end], result.nodes.back());

    return result;
}

node::NetsSolver::NetSolution node::NetsSolver::SolveL()
{
    using namespace node;
    using namespace node::model;

    NetSolution result;
    bool start_is_left = m_start_node->point.x < m_end_node->point.x;

    if (start_is_left)
    {
        auto net_string = NetUtils::CreateStringNet(NetUtils::StringNetDescription
            {
                {m_start_node->point, NetSegmentOrientation::vertical},
                {{m_start_node->point.x, m_end_node->point.y}, NetSegmentOrientation::horizontal},
                {m_end_node->point, {}}
            });
        result.nodes = std::move(net_string.nodes);
        result.segments = std::move(net_string.segments);
        result.start = result.nodes[0].GetId();
        result.end = result.nodes.back().GetId();
        return result;
    }
    else
    {
        auto net_string = NetUtils::CreateStringNet(NetUtils::StringNetDescription
            {
                {m_start_node->point, NetSegmentOrientation::horizontal},
                {{m_end_node->point.x, m_start_node->point.y}, NetSegmentOrientation::vertical},
                {m_end_node->point, {}}
            });
        result.nodes = std::move(net_string.nodes);
        result.segments = std::move(net_string.segments);
        result.start = result.nodes[0].GetId();
        result.end = result.nodes.back().GetId();
        return result;
    }
}

node::NetsSolver::NetSolution node::NetsSolver::SolveHFlipL()
{
    using namespace node;
    using namespace node::model;

    NetSolution result;
    bool start_is_left = m_start_node->point.x < m_end_node->point.x;

    if (start_is_left)
    {
        auto net_string = NetUtils::CreateStringNet(NetUtils::StringNetDescription
            {
                {m_start_node->point, NetSegmentOrientation::horizontal},
                {{m_end_node->point.x, m_start_node->point.y}, NetSegmentOrientation::vertical},
                {m_end_node->point, {}}
            });
        result.nodes = std::move(net_string.nodes);
        result.segments = std::move(net_string.segments);
        result.start = result.nodes[0].GetId();
        result.end = result.nodes.back().GetId();
        return result;
    }
    else
    {
        auto net_string = NetUtils::CreateStringNet(NetUtils::StringNetDescription
            {
                {m_start_node->point, NetSegmentOrientation::vertical},
                {{m_start_node->point.x, m_end_node->point.y}, NetSegmentOrientation::horizontal},
                {m_end_node->point, {}}
            });
        result.nodes = std::move(net_string.nodes);
        result.segments = std::move(net_string.segments);
        result.start = result.nodes[0].GetId();
        result.end = result.nodes.back().GetId();
        return result;
    }
}

node::NetsSolver::NetSolution node::NetsSolver::ExtendOneThenSolve(model::ConnectedSegmentSide side)
{
    using namespace node;
    using namespace node::model;
    NetSolution result;

    if (side == ConnectedSegmentSide::south)
    {
        bool start_is_lower = m_start_node->point.y <= m_end_node->point.y;

        if (start_is_lower)
        {
            // add start node
            result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_start_node->point });

            // add segment between start and inner solve
            result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
                NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::vertical });
        }

        auto inner_solver{ CreateSimilarSolver() };

        if (start_is_lower)
        {
            inner_solver.SetStartDescription(NetSolutionEndDescription{ {m_start_node->point.x, m_start_node->point.y + GetExtensionDistance()},
                {false, true, true, true} });
            inner_solver.SetEndDescription(*m_end_node);
        }
        else
        {
            inner_solver.SetStartDescription(*m_start_node);
            inner_solver.SetEndDescription(
                NetSolutionEndDescription{ {m_end_node->point.x, m_end_node->point.y + GetExtensionDistance()},
                    {false, true, true, true} }
            );
        }
        auto inner_result = inner_solver.Solve();

        auto inner_remap = AddInnerResult(inner_result, result);

        if (start_is_lower)
        {
            NetUtils::connectSegementAndNodes(result.segments[0], result.nodes[0], result.nodes[1]);
        }
        else
        {
            // add end node
            result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_end_node->point });

            // add segment between end and inner solve
            result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
                NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::vertical });

            NetUtils::connectSegementAndNodes(result.segments.back(), result.nodes[inner_remap.end], result.nodes.back());
        }
        result.start = result.nodes[0].GetId();
        result.end = result.nodes.back().GetId();
    }
    else if (side == ConnectedSegmentSide::east)
    {
        bool start_is_right = m_start_node->point.x >= m_end_node->point.x;

        if (start_is_right)
        {
            // add start node
            result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_start_node->point });

            // add segment between start and inner solve
            result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
                NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::horizontal });
        }

        auto inner_solver{ CreateSimilarSolver() };

        if (start_is_right)
        {
            inner_solver.SetStartDescription(NetSolutionEndDescription{ {m_start_node->point.x + GetExtensionDistance(), m_start_node->point.y},
                {true, true, true, false} });
            inner_solver.SetEndDescription(*m_end_node);
        }
        else
        {
            inner_solver.SetStartDescription(*m_start_node);
            inner_solver.SetEndDescription(
                NetSolutionEndDescription{ {m_end_node->point.x + GetExtensionDistance(), m_end_node->point.y},
                    {true, true, true, false} }
            );
        }
        auto inner_result = inner_solver.Solve();

        auto inner_remap = AddInnerResult(inner_result, result);

        if (start_is_right)
        {
            NetUtils::connectSegementAndNodes(result.segments[0], result.nodes[0], result.nodes[1]);
        }
        else
        {
            // add end node
            result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_end_node->point });

            // add segment between end and inner solve
            result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
                NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::horizontal });

            NetUtils::connectSegementAndNodes(result.segments.back(), result.nodes[inner_remap.end], result.nodes.back());
        }
        result.start = result.nodes[0].GetId();
        result.end = result.nodes.back().GetId();
    }
    else
    {
        assert(false); // unimplemented
    }
    return result;
}

node::NetsSolver::NetSolution node::NetsSolver::ExtendTwoThenSolve(model::NetSegmentOrientation orientation)
{
    using namespace node;
    using namespace node::model;

    NetSolution result;
    if (orientation == model::NetSegmentOrientation::horizontal)
    {
        bool start_is_left = m_start_node->point.x <= m_end_node->point.x;

        // add start node
        result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_start_node->point });

        // add segment between start and inner solve
        result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
            NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::horizontal });

        auto inner_solver{ CreateSimilarSolver() };

        if (start_is_left)
        {
            inner_solver.SetStartDescription(NetSolutionEndDescription{ {m_start_node->point.x - GetExtensionDistance(), m_start_node->point.y},
                {true, false, true, true} });
            inner_solver.SetEndDescription(NetSolutionEndDescription{ {m_end_node->point.x + GetExtensionDistance(), m_end_node->point.y},
                {true, true, true, false} });
        }
        else
        {
            inner_solver.SetStartDescription(NetSolutionEndDescription{ {m_start_node->point.x + GetExtensionDistance(), m_start_node->point.y},
                {true, true, true, false} });
            inner_solver.SetEndDescription(NetSolutionEndDescription{ {m_end_node->point.x - GetExtensionDistance(), m_end_node->point.y},
                {true, false, true, true} });
        }
        auto inner_result = inner_solver.Solve();

        auto inner_remap = AddInnerResult(inner_result, result);

        NetUtils::connectSegementAndNodes(result.segments[0], result.nodes[0], result.nodes[1]);

        // add end node
        result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_end_node->point });

        // add segment between end and inner solve
        result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
            NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::horizontal });

        NetUtils::connectSegementAndNodes(result.segments.back(), result.nodes[inner_remap.end], result.nodes.back());
        
        result.start = result.nodes[0].GetId();
        result.end = result.nodes.back().GetId();
    }
    else // vertical
    {
        bool start_is_top = m_start_node->point.y <= m_end_node->point.y;

        // add start node
        result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_start_node->point });

        // add segment between start and inner solve
        result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
            NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::vertical });

        auto inner_solver{ CreateSimilarSolver() };

        if (start_is_top)
        {
            inner_solver.SetStartDescription(NetSolutionEndDescription{ {m_start_node->point.x, m_start_node->point.y - GetExtensionDistance()},
                {true, true, false, true} });
            inner_solver.SetEndDescription(NetSolutionEndDescription{ {m_end_node->point.x, m_end_node->point.y + GetExtensionDistance()},
                {false, true, true, true} });
        }
        else
        {
            inner_solver.SetStartDescription(NetSolutionEndDescription{ {m_start_node->point.x, m_start_node->point.y + GetExtensionDistance()},
                {false, true, true, true} });
            inner_solver.SetEndDescription(NetSolutionEndDescription{ {m_end_node->point.x, m_end_node->point.y - GetExtensionDistance()},
                {true, true, false, true} });
        }
        auto inner_result = inner_solver.Solve();

        auto inner_remap = AddInnerResult(inner_result, result);

        NetUtils::connectSegementAndNodes(result.segments[0], result.nodes[0], result.nodes[1]);

        // add end node
        result.nodes.push_back(NetNodeModel{ GetNewNodeId(), m_end_node->point });

        // add segment between end and inner solve
        result.segments.push_back(NetSegmentModel{ GetNewSegmentId(),
            NetNodeId{0}, NetNodeId{0}, model::NetSegmentOrientation::vertical });

        NetUtils::connectSegementAndNodes(result.segments.back(), result.nodes[inner_remap.end], result.nodes.back());

        result.start = result.nodes[0].GetId();
        result.end = result.nodes.back().GetId();
    }
    return result;
}

node::NetsSolver node::NetsSolver::CreateSimilarSolver() const
{
    NetsSolver inner_solver = *this;
    inner_solver.Reset();
    return inner_solver;
}

node::model::NetNodeId node::NetsSolver::GetNewNodeId()
{
    return model::NetNodeId{m_new_node_id++};
}

node::model::NetSegmentId node::NetsSolver::GetNewSegmentId()
{
    return model::NetSegmentId{ m_new_segment_id++};
}
