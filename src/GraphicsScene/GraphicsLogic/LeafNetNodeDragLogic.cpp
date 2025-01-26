#include "LeafNetNodeDragLogic.hpp"
#include "NetObject.hpp"
#include "GraphicsObjectsManager.hpp"
#include "BlockObject.hpp"
#include "BlockSocketObject.hpp"
#include "NetUtils/NetsSolver.hpp"
#include "GraphicsScene/SolverUtils.hpp"



std::unique_ptr<node::logic::LeafNetNodeDragLogic> node::logic::LeafNetNodeDragLogic::TryCreate(NetNode& leaf_node,
	GraphicsScene& scene, GraphicsObjectsManager& manager)
{
	assert(leaf_node.GetConnectedSegmentsCount() == 1);
	TemporaryNetManager net_mgr = TemporaryNetManager::CreateFromLeafNodeNet(leaf_node, scene);

	return std::make_unique<LeafNetNodeDragLogic>(std::move(net_mgr), &scene, &manager);
}

node::logic::LeafNetNodeDragLogic::LeafNetNodeDragLogic(TemporaryNetManager net_manager, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{ scene, manager } , m_temp_net_mgr{std::move(net_manager)}
{
	PositionNodes(m_temp_net_mgr.orig_nodes.back()->getCenter());
}

void node::logic::LeafNetNodeDragLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	PositionNodes(current_mouse_point);
}

MI::ClickEvent node::logic::LeafNetNodeDragLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	CleanUp();
	if (!std::visit(AnchorAlive{}, m_temp_net_mgr.start_anchor))
	{
		return MI::ClickEvent::CAPTURE_END;
	}
	const auto* end_socket = GetSocketAt(current_mouse_point);
	if (end_socket && end_socket == m_temp_net_mgr.orig_nodes.back()->GetConnectedSocket())
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	assert(GetObjectsManager());

	auto request = PopulateResultNet(current_mouse_point);
	GetObjectsManager()->GetSceneModel()->UpdateNet(std::move(request));

	return MI::ClickEvent::CAPTURE_END;
}



void node::logic::LeafNetNodeDragLogic::OnCancel()
{
	CleanUp();
}

node::BlockSocketObject* node::logic::LeafNetNodeDragLogic::GetSocketAt(const model::Point& point) const
{
	const auto& blocks = GetObjectsManager()->getBlocksRegistry();

	auto current_mouse_point_SDL = ToSDLPoint(point);
	BlockSocketObject* end_socket = nullptr;
	for (const auto& block_it : blocks)
	{
		const auto& sockets = block_it.second->GetSockets();
		auto it = std::find_if(sockets.begin(), sockets.end(), [&](const auto& socket)
			{
				auto socket_rect = ToSDLRect(socket->GetSceneRect());
				if (SDL_PointInRectFloat(&current_mouse_point_SDL, &socket_rect))
				{
					return true;
				}
				return false;
			});
		if (it != sockets.end())
		{
			auto* connected_node = it->get()->GetConnectedNode();
			if (connected_node == nullptr || connected_node == m_temp_net_mgr.orig_nodes.back().GetObjectPtr())
			{
				end_socket = it->get();
				break;
			}
		}
	}
	return end_socket;
}

void node::logic::LeafNetNodeDragLogic::ResetNodes()
{
	m_temp_net_mgr.ResetNodes();
}

void node::logic::LeafNetNodeDragLogic::PositionNodes(const model::Point& target_point)
{
	BlockSocketObject* end_socket = GetSocketAt(target_point);

	auto end_point = target_point;
	if (end_socket)
	{
		end_point = end_socket->GetCenterInSpace();
	}

	ResetNodes();

	NetSolutionEndDescription end_descriptor{};
	if (end_socket)
	{
		std::array<bool, 4> sides{};
		sides[static_cast<int>(end_socket->GetConnectionSide())] = true;
		end_descriptor = NetSolutionEndDescription{ end_point, sides };
	}
	else
	{
		end_descriptor = NetSolutionEndDescription{ end_point, {true, true, true, true} };
	}
	m_temp_net_mgr.PositionNodes(end_descriptor);
}

void node::logic::LeafNetNodeDragLogic::CleanUp()
{
	m_temp_net_mgr.CleanUp();
}

node::NetModificationRequest node::logic::LeafNetNodeDragLogic::PopulateResultNet(const model::Point& current_mouse_point)
{
	using model::NetNodeId;
	using model::NetSegmentId;
	using enum model::ConnectedSegmentSide;

	const auto* end_socket = GetSocketAt(current_mouse_point);

	NetsSolver solver;

	model::Point start = std::visit(AnchorStart{}, m_temp_net_mgr.start_anchor);

	solver.SetStartDescription(NetSolutionEndDescription{ start, std::visit(AnchorGetConnectionSide{}, m_temp_net_mgr.start_anchor) });
	if (end_socket)
	{
		std::array<bool, 4> sides{};
		sides[static_cast<int>(end_socket->GetConnectionSide())] = true;
		solver.SetEndDescription(NetSolutionEndDescription{ end_socket->GetCenterInSpace(), sides });
	}
	else
	{
		solver.SetEndDescription(NetSolutionEndDescription{ current_mouse_point, {true, true, true, true} });
	}
	const auto solution = solver.Solve();
	auto report = MakeModificationsReport(solution, m_temp_net_mgr.orig_nodes, m_temp_net_mgr.orig_segments);
	UpdateModificationEndWithSocket(m_temp_net_mgr.orig_nodes, report, end_socket);
	return std::move(report.request);
}

