#include "GraphicsLogic/NewNetLogic.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"
#include "NetUtils/NetsSolver.hpp"
#include "GraphicsScene/SolverUtils.hpp"


std::unique_ptr<node::logic::NewNetLogic> node::logic::NewNetLogic::CreateFromSocket(BlockSocketObject& socket, 
	GraphicsScene* scene, GraphicsObjectsManager* manager)
{
	assert(scene);
	assert(manager);
	TemporaryNetManager net = TemporaryNetManager::Create(*scene);
	net.start_anchor = SocketAnchor{ HandlePtrS<BlockSocketObject,GraphicsObject>{socket}, socket.GetCenterInSpace() };
	return std::make_unique<NewNetLogic>(std::move(net), scene, manager);

}

std::unique_ptr<node::logic::NewNetLogic> node::logic::NewNetLogic::CreateFromSegment(NetSegment& base_segment, const model::Point& start_point, GraphicsScene* scene, GraphicsObjectsManager* manager)
{
	assert(scene);
	assert(manager);
	TemporaryNetManager net = TemporaryNetManager::Create(*scene);
	net.start_anchor = SegmentAnchor{ HandlePtrS<NetSegment, GraphicsObject>{base_segment}, {base_segment.getStartNode()->getCenter().x, start_point.y} };
	return std::make_unique<NewNetLogic>(std::move(net), scene, manager);

}

node::logic::NewNetLogic::NewNetLogic(TemporaryNetManager&& net, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager }, m_net{std::move(net)}
{
	ResetNodes();
	auto pos = std::visit(AnchorStart{}, m_net.start_anchor);
	PositionNodes(pos);
}

void node::logic::NewNetLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	PositionNodes(current_mouse_point);
}

MI::ClickEvent node::logic::NewNetLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	if (!std::visit(AnchorAlive{}, m_net.start_anchor))
	{
		CleanUp();
		return MI::ClickEvent::CAPTURE_END;
	}

	assert(GetObjectsManager());
	CleanUp();

	model::Point start = std::visit(AnchorStart{}, m_net.start_anchor);

	if (std::abs(start.x - current_mouse_point.x) >= 5 || std::abs(start.y - current_mouse_point.y) >= 5)
	{
		auto request = PopulateResultNet(current_mouse_point);
		GetObjectsManager()->GetSceneModel()->UpdateNet(std::move(request));
	}

	return MI::ClickEvent::CAPTURE_END;
}

void node::logic::NewNetLogic::OnCancel()
{
	CleanUp();
}

void node::logic::NewNetLogic::ResetNodes()
{
	m_net.ResetNodes();
}

void node::logic::NewNetLogic::PositionNodes(const model::Point& target_point)
{
	BlockSocketObject* end_socket = GetSocketAt(target_point);
	NetSegment* end_segment = nullptr;
	if (!end_socket)
	{
		end_segment = GetSegmentAt(target_point);
	}

	model::Point end_point = target_point;
	if (end_socket)
	{
		m_net.ResetNodes();
		end_point = end_socket->GetCenterInSpace();
	}
	if (end_segment)
	{
		auto segment_center = end_segment->GetCenter();
		if (end_segment->GetOrientation() == model::NetSegmentOrientation::vertical)
		{
			end_point.x = segment_center.x;
		}
		else
		{
			end_point.y = segment_center.y;
		}
	}

	if (!std::visit(AnchorAlive{}, m_net.start_anchor))
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return;
	}
	
	NetSolutionEndDescription end_descr = NetSolutionEndDescription{ end_point, {true,true,true,true} };
	if (end_socket)
	{
		end_descr.allowed_sides = { false,false,false,false };
		end_descr.allowed_sides[static_cast<size_t>(end_socket->GetConnectionSide())] = true;
	}
	if (end_segment)
	{
		if (end_segment->GetOrientation() == model::NetSegmentOrientation::vertical)
		{
			end_descr.allowed_sides = { false, true, false, true };
		}
		else
		{
			end_descr.allowed_sides = { true, false ,true, false };
		}
	}

	m_net.PositionNodes(end_descr);
}

node::NetModificationRequest node::logic::NewNetLogic::PopulateResultNet(const model::Point& current_mouse_point)
{
	using model::NetNodeId;
	using model::NetSegmentId;
	using enum model::ConnectedSegmentSide;

	const auto* end_socket = GetSocketAt(current_mouse_point);
	NetSegment* end_segment = nullptr;
	if (!end_socket)
	{
		end_segment = GetSegmentAt(current_mouse_point);
	}

	NetsSolver solver;

	model::Point start = std::visit(AnchorStart{}, m_net.start_anchor);

	solver.SetStartDescription(NetSolutionEndDescription{ start, std::visit(AnchorGetConnectionSide{}, m_net.start_anchor) });
	if (end_socket)
	{
		std::array<bool, 4> sides{};
		sides[static_cast<int>(end_socket->GetConnectionSide())] = true;
		solver.SetEndDescription(NetSolutionEndDescription{ end_socket->GetCenterInSpace(), sides});
	}
	else if (end_segment)
	{
		if (end_segment->GetOrientation() == model::NetSegmentOrientation::vertical)
		{
			model::Point end_point = current_mouse_point;
			end_point.x = end_segment->GetCenter().x;
			solver.SetEndDescription({ end_point,{ false, true, false, true } });
		}
		else
		{
			model::Point end_point = current_mouse_point;
			end_point.y = end_segment->GetCenter().y;
			solver.SetEndDescription({ end_point,{ true, false, true, false} });
		}
	}
	else
	{
		solver.SetEndDescription(NetSolutionEndDescription{ current_mouse_point, {true, true, true, true} });
	}
	const auto solution = solver.Solve();

	NetModificationRequest request = MakeCreationReport(solution);

	BlockSocketObject* start_socket = nullptr;
	if (std::holds_alternative<SocketAnchor>(m_net.start_anchor))
	{
		start_socket = std::get<SocketAnchor>(m_net.start_anchor).socket.GetObjectPtr();
		if (start_socket)
		{
			assert(start_socket->GetId());
			assert(start_socket->GetParentBlock());
			assert(start_socket->GetParentBlock()->GetModelId());

			request.added_connections.push_back(NetModificationRequest::SocketConnectionRequest{
				model::SocketUniqueId{*(start_socket->GetId()), *(start_socket->GetParentBlock()->GetModelId())},
				NetModificationRequest::NodeIdType::new_id,
				NetNodeId{0}
				});
		}
	}
	else if (std::holds_alternative<SegmentAnchor>(m_net.start_anchor))
	{
		auto* start_segment = std::get<SegmentAnchor>(m_net.start_anchor).segment.GetObjectPtr();
		if (start_segment)
		{
			auto* start_node = start_segment->getStartNode();
			auto* end_node = start_segment->getEndNode();
			if (start_node->getCenter().y > end_node->getCenter().y)
			{
				std::swap(start_node, end_node); // make sure start is on top
			}
			assert(start_node->GetId());
			assert(end_node->GetId());
			request.update_segments.push_back(NetModificationRequest::UpdateSegmentRequest{
				NetModificationRequest::NodeIdType::existing_id,
				NetModificationRequest::NodeIdType::new_id,
				model::ConnectedSegmentSide::south,
				model::ConnectedSegmentSide::north,
				model::NetSegmentOrientation::vertical,
				*start_segment->GetId(),
				*start_node->GetId(),
				NetNodeId{0}
				});
			request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
				NetModificationRequest::NodeIdType::new_id,
				NetModificationRequest::NodeIdType::existing_id,
				model::ConnectedSegmentSide::south,
				model::ConnectedSegmentSide::north,
				model::NetSegmentOrientation::vertical,
				model::NetNodeId{0},
				*end_node->GetId()
				});
		}
	}

	if (end_socket && end_socket != start_socket)
	{
		request.added_connections.push_back(NetModificationRequest::SocketConnectionRequest{
			model::SocketUniqueId{*(end_socket->GetId()), *(end_socket->GetParentBlock()->GetModelId())},
			NetModificationRequest::NodeIdType::new_id,
			solution.nodes.back().GetId()
			});
	}
	if (end_segment)
	{
		UpdateModificationEndWithSegment({}, request, 
			NetNodeModificationInfo{ NetModificationRequest::NodeIdType::new_id, 
			NetNodeId{static_cast<model::id_int>(request.added_nodes.size() - 1)},  request.added_nodes.size() - 1 }, 
			end_segment);
	}
	return request;
}

node::BlockSocketObject* node::logic::NewNetLogic::GetSocketAt(const model::Point& point) const
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
		const GraphicsObject* owned_segment = nullptr;
		if (std::holds_alternative<SocketAnchor>(m_net.start_anchor))
		{
			owned_segment = std::get<SocketAnchor>(m_net.start_anchor).socket.GetObjectPtr();
		}
		if (it != sockets.end() && owned_segment != it->get())
		{
			end_socket = it->get();
			break;
		}
	}
	return end_socket;
}

node::NetSegment* node::logic::NewNetLogic::GetSegmentAt(const model::Point& point) const
{
	const auto& segments = GetObjectsManager()->getSegmentsRegistry();

	auto current_mouse_point_SDL = ToSDLPoint(point);
	NetSegment* end_segment = nullptr;
	for (const auto& segment_it : segments)
	{
		if (!segment_it.second->IsVisible())
		{
			continue;
		}
		auto segment_rect = ToSDLRect(segment_it.second->GetSceneRect());
		if (!SDL_PointInRectFloat(&current_mouse_point_SDL, &segment_rect))
		{
			continue;
		}

		end_segment = segment_it.second;
	}
	return end_segment;
}

void node::logic::NewNetLogic::CleanUp()
{
	m_net.CleanUp();
}

