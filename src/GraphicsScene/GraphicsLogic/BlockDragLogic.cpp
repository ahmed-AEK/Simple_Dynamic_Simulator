#include "BlockDragLogic.hpp"
#include "BlockObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/SolverUtils.hpp"

std::unique_ptr<node::logic::BlockDragLogic> node::logic::BlockDragLogic::TryCreate(
    model::Point startPointMouseSpace, BlockObject& block, 
    GraphicsScene* scene, GraphicsObjectsManager* manager)
{
    std::unordered_map<model::SocketId, TemporaryNetManager> socket_nets;
    assert(scene);
    for (const auto& socket : block.GetSockets())
    {
        assert(socket->GetId());
        if (!socket->GetId())
        {
            return nullptr;
        }
		auto* current_node = socket->GetConnectedNode();
		if (!current_node)
		{
            continue;
		}
        TemporaryNetManager net = TemporaryNetManager::CreateFromLeafNodeNet(*current_node, *scene);
        net.SetHighlight(false);
        socket_nets[*socket->GetId()] = std::move(net);
    }

    return std::make_unique<BlockDragLogic>(startPointMouseSpace,
        block.GetScenePosition(), block, std::move(socket_nets), scene, manager);
}

node::logic::BlockDragLogic::BlockDragLogic(model::Point startPointMouseSpace,
    model::Point startObjectEdge, BlockObject& block, 
    std::unordered_map<model::SocketId, TemporaryNetManager> socket_nets,
    GraphicsScene* scene, GraphicsObjectsManager* manager)
    :GraphicsLogic(scene, manager), m_startPointMouseSpace{ startPointMouseSpace }, 
    m_startObjectEdge{ startObjectEdge }, m_block{ block }, m_socket_nets{std::move(socket_nets)}
{
    assert(scene);
    assert(manager);

    PositionNodes(block.GetPosition());
}

void node::logic::BlockDragLogic::OnMouseMove(const model::Point& current_mouse_point)
{
    auto drag_object = m_block.GetObjectPtr();
    if (!drag_object)
    {
        GetScene()->SetGraphicsLogic(nullptr);
        return;
    }


    const auto drag_vector = model::Point{ 
        current_mouse_point.x - m_startPointMouseSpace.x, 
        current_mouse_point.y - m_startPointMouseSpace.y };


    drag_object->SetPosition(GetScene()->QuantizePoint({
        m_startObjectEdge.x + drag_vector.x,
        m_startObjectEdge.y + drag_vector.y
        }));
    PositionNodes(drag_object->GetPosition());
}

void node::logic::BlockDragLogic::OnCancel()
{
    auto drag_object = m_block.GetObjectPtr();
    if (!drag_object)
    {
        return;
    }
    drag_object->SetPosition(GetScene()->QuantizePoint({
        m_startObjectEdge.x,
        m_startObjectEdge.y
        }));

}

MI::ClickEvent node::logic::BlockDragLogic::OnLMBUp(const model::Point& current_mouse_point)
{
    auto* scene = GetScene();
    auto* block = m_block.GetObjectPtr();
    if (!scene && !block)
    {
        return MI::ClickEvent::CAPTURE_END;
    }

    const auto drag_vector = model::Point{
    current_mouse_point.x - m_startPointMouseSpace.x,
    current_mouse_point.y - m_startPointMouseSpace.y };

    CleanUp();

    // nothing to do if drag is zero.
    model::Point quantized_drag_vector = scene->QuantizePoint(drag_vector + m_startObjectEdge);
    if (quantized_drag_vector == m_startObjectEdge)
    {
        return MI::ClickEvent::CAPTURE_END;
    }

    auto request = PopulateResultNet(quantized_drag_vector);
    
    assert(GetObjectsManager());
    auto block_id = block->GetModelId();
    assert(block_id);
    GetObjectsManager()->GetSceneModel()->MoveBlockById(
        *block_id, scene->QuantizePoint(drag_vector + m_startObjectEdge), std::move(request));

    return MI::ClickEvent::CAPTURE_END;
}

void node::logic::BlockDragLogic::ResetNodes()
{
    for (auto& [id, net] : m_socket_nets)
    {
        net.ResetNodes();
    }
}

void node::logic::BlockDragLogic::CleanUp()
{
    m_block->SetPosition(GetScene()->QuantizePoint({
        m_startObjectEdge.x,
        m_startObjectEdge.y
    }));

    for (auto& [id, net] : m_socket_nets)
    {
        net.CleanUp();
    }
}

void node::logic::BlockDragLogic::PositionNodes(const model::Point& target_point)
{
    for (auto&& [id, net] : m_socket_nets)
    {
        auto* socket = m_block->GetSocketById(id);
        
        auto end_point = target_point + socket->GetCenterInBlock();

        std::array<bool, 4> sides{};
        sides[static_cast<int>(socket->GetConnectionSide())] = true;

        net.PositionNodes(NetSolutionEndDescription{ end_point, sides });

    }
}


node::NetModificationRequest node::logic::BlockDragLogic::PopulateResultNet(const model::Point& current_block_point)
{
    using model::NetNodeId;
    using model::NetSegmentId;
    using enum model::ConnectedSegmentSide;
    NetModificationRequest result;
    for (auto&& [socket_id, net] : m_socket_nets)
    {

        const auto* end_socket = m_block->GetSocketById(socket_id);

        NetsSolver solver;

        model::Point start = std::visit(AnchorStart{}, net.start_anchor);

        solver.SetStartDescription(NetSolutionEndDescription{ start, std::visit(AnchorGetConnectionSide{}, net.start_anchor) });

        std::array<bool, 4> sides{};
        sides[static_cast<int>(end_socket->GetConnectionSide())] = true;
        solver.SetEndDescription(NetSolutionEndDescription{ current_block_point + end_socket->GetCenterInBlock(), sides });

        const auto solution = solver.Solve();
        auto report = MakeModificationsReport(solution, net.orig_nodes, net.orig_segments);
        UpdateModificationEndWithSocket(net.orig_nodes, report, end_socket);

        auto net_id_opt = net.orig_nodes.back()->GetNetId();
        assert(net_id_opt);
        for (auto& added_node : report.request.added_nodes)
        {
            added_node.net_id = *net_id_opt;
            added_node.net_type = NetModificationRequest::IdType::existing_id;
        }

        MergeModificationRequests(report.request, result);
    }

    return result;
}


