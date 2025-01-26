#include "GraphicsLogic/NewNetLogic.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"
#include "NetUtils/NetsSolver.hpp"
#include "GraphicsScene/SolverUtils.hpp"

#include <array>
#include <algorithm>

std::unique_ptr<node::logic::NewNetLogic> node::logic::NewNetLogic::CreateFromSocket(BlockSocketObject& socket, 
	GraphicsScene* scene, GraphicsObjectsManager* manager)
{
	assert(scene);
	std::array<NetNode*, 6> nodes{};
	std::array<NetSegment*, 5> segments{};
	try
	{
		for (auto&& segment : segments)
		{
			auto new_segmet = std::make_unique<NetSegment>(model::NetSegmentOrientation::vertical, nullptr, nullptr);
			segment = new_segmet.get();
			scene->AddObject(std::move(new_segmet), GraphicsScene::SegmentLayer);
			segment->SetSelected(true);
		}
		for (auto&& node : nodes)
		{
			auto new_node = std::make_unique<NetNode>(model::Point{ 0,0 });
			node = new_node.get();
			scene->AddObject(std::move(new_node), GraphicsScene::NetNodeLayer);
			node->SetSelected(true);
		}
		assert(manager);
		return std::make_unique<NewNetLogic>(SocketAnchor{ HandlePtrS<BlockSocketObject,GraphicsObject>{socket}, socket.GetCenterInSpace() }, nodes, segments, scene, manager);
	}
	catch (...)
	{
		// if we failed to create the object, delete everything
		for (const auto* node : nodes)
		{
			if (node)
			{
				scene->PopObject(node);
			}
		}
		for (const auto* segment : segments)
		{
			if (segment)
			{
				scene->PopObject(segment);
			}
		}
	}
	return nullptr;
}

std::unique_ptr<node::logic::NewNetLogic> node::logic::NewNetLogic::CreateFromSegment(NetSegment& base_segment, const model::Point& start_point, GraphicsScene* scene, GraphicsObjectsManager* manager)
{
	assert(scene);
	std::array<NetNode*, 6> nodes{};
	std::array<NetSegment*, 5> segments{};
	try
	{
		int layer = 1000;
		for (auto&& segment : segments)
		{
			auto new_segmet = std::make_unique<NetSegment>(model::NetSegmentOrientation::vertical, nullptr, nullptr);
			segment = new_segmet.get();
			scene->AddObject(std::move(new_segmet), GraphicsScene::SegmentLayer + layer);
			layer++;
		}
		for (auto&& node : nodes)
		{
			auto new_node = std::make_unique<NetNode>(model::Point{ 0,0 });
			node = new_node.get();
			scene->AddObject(std::move(new_node), GraphicsScene::NetNodeLayer + layer);
			layer++;
		}
		assert(manager);
		return std::make_unique<NewNetLogic>(SegmentAnchor{ HandlePtrS<NetSegment, GraphicsObject>{base_segment}, {base_segment.getStartNode()->getCenter().x, start_point.y} }, nodes, segments, scene, manager);
	}
	catch (...)
	{
		// if we failed to create the object, delete everything
		for (const auto* node : nodes)
		{
			if (node)
			{
				scene->PopObject(node);
			}
		}
		for (const auto* segment : segments)
		{
			if (segment)
			{
				scene->PopObject(segment);
			}
		}
	}
	return nullptr;
}

node::logic::NewNetLogic::NewNetLogic(anchor_t start_anchor, std::array<NetNode*, 6> nodes,
	std::array<NetSegment*, 5> segments, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager }, m_start_anchor{std::move(start_anchor)}
{
	
	std::transform(nodes.begin(), nodes.end(), m_nodes.begin(), 
		[](const auto& node)
		{
			assert(node);
			return HandlePtrS<NetNode, GraphicsObject>{*node};
		});
	std::transform(segments.begin(), segments.end(), m_segments.begin(),
		[](const auto& segment)
		{
			assert(segment);
			return HandlePtrS<NetSegment, GraphicsObject>{*segment};
		});
	for (auto&& node : nodes)
	{
		node->setCenter(std::visit([](const auto& anchor) {return anchor.position; }, m_start_anchor));
	}
	ResetNodes();

	for (int i = 0; i < 4; i++)
	{
		nodes[i]->SetVisible(true);
	}
	for (int i = 0; i < 3; i++)
	{
		segments[i]->SetVisible(true);
	}

	segments[0]->Connect(nodes[0], nodes[1], model::NetSegmentOrientation::horizontal);
	segments[1]->Connect(nodes[1], nodes[2], model::NetSegmentOrientation::vertical);
	segments[2]->Connect(nodes[2], nodes[3], model::NetSegmentOrientation::horizontal);
}

namespace
{

struct AnchorAlive
{
	bool operator()(const node::logic::NewNetLogic::SocketAnchor& socket)
	{
		return socket.socket.isAlive();
	}
	bool operator()(const node::logic::NewNetLogic::SegmentAnchor& segment)
	{
		return segment.segment.isAlive();
	}
};

struct AnchorGetConnectionSide
{
	std::array<bool,4> operator()(const node::logic::NewNetLogic::SocketAnchor& socket)
	{
		std::array<bool, 4> sides{};
		sides[static_cast<int>(socket.socket->GetConnectionSide())] = true;
		return sides;
	}
	std::array<bool,4> operator()(const node::logic::NewNetLogic::SegmentAnchor& segment)
	{
		std::array<bool, 4> sides{};
		if (segment.segment->GetOrientation() == node::model::NetSegmentOrientation::horizontal)
		{
			sides[0] = true;
			sides[2] = true;
		}
		else
		{
			sides[1] = true;
			sides[3] = true;
		}
		return sides;
	}
};

}


void node::logic::NewNetLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	BlockSocketObject* end_socket = GetSocketAt(current_mouse_point);
	
	auto end_point = current_mouse_point;
	if (end_socket)
	{
		end_point = end_socket->GetCenterInSpace();
	}

	if (!std::visit(AnchorAlive{}, m_start_anchor))
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return;
	}

	model::Point start = std::visit([](const auto& anchor) {return anchor.position; }, m_start_anchor);

	ResetNodes();

	if (std::abs(start.x - end_point.x) < 5 && std::abs(start.y - end_point.y) < 5)
	{
		for (int i = 0; i < 4; i++)
		{
			m_nodes[i]->SetVisible(true);
		}
		for (int i = 0; i < 3; i++)
		{
			m_segments[i]->SetVisible(true);
		}

		model::node_int midpoint_x = (end_point.x + start.x) / 2;
		m_nodes[1]->setCenter({ midpoint_x, start.y });
		m_nodes[2]->setCenter({ midpoint_x, end_point.y });
		m_nodes[3]->setCenter({ end_point.x, end_point.y });
		m_segments[0]->CalcRect();
		m_segments[1]->CalcRect();
		m_segments[2]->CalcRect();
	}
	else
	{
		NetsSolver solver;
		solver.SetStartDescription(NetSolutionEndDescription{ start, std::visit(AnchorGetConnectionSide{}, m_start_anchor) });
		if (end_socket)
		{
			std::array<bool, 4> sides{};
			sides[static_cast<int>(end_socket->GetConnectionSide())] = true;
			solver.SetEndDescription(NetSolutionEndDescription{ end_point, sides});
		}
		else
		{
			solver.SetEndDescription(NetSolutionEndDescription{ end_point, {true, true, true, true} });
		}
		auto solution = solver.Solve();
		ApplySolutionToNodes(solution, m_nodes, m_segments);
	}
}

MI::ClickEvent node::logic::NewNetLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	if (!std::visit(AnchorAlive{}, m_start_anchor))
	{
		CleanUp();
		return MI::ClickEvent::CAPTURE_END;
	}

	assert(GetObjectsManager());
	CleanUp();

	model::Point start = std::visit([](const auto& anchor) {return anchor.position; }, m_start_anchor);

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
	for (auto& node : m_nodes)
	{
		node->SetVisible(false);
	}
	for (auto& segment : m_segments)
	{
		segment->SetVisible(false);
		segment->Disconnect();
	}
}

node::NetModificationRequest node::logic::NewNetLogic::PopulateResultNet(const model::Point& current_mouse_point)
{
	using model::NetNodeId;
	using model::NetSegmentId;
	using enum model::ConnectedSegmentSide;

	const auto* end_socket = GetSocketAt(current_mouse_point);

	NetsSolver solver;

	model::Point start = std::visit([](const auto& anchor) {return anchor.position; }, m_start_anchor);

	solver.SetStartDescription(NetSolutionEndDescription{ start, std::visit(AnchorGetConnectionSide{}, m_start_anchor) });
	if (end_socket)
	{
		std::array<bool, 4> sides{};
		sides[static_cast<int>(end_socket->GetConnectionSide())] = true;
		solver.SetEndDescription(NetSolutionEndDescription{ end_socket->GetCenterInSpace(), sides});
	}
	else
	{
		solver.SetEndDescription(NetSolutionEndDescription{ current_mouse_point, {true, true, true, true} });
	}
	const auto solution = solver.Solve();

	NetModificationRequest request;
	
	for (const auto& node : solution.nodes)
	{
		request.added_nodes.push_back(NetModificationRequest::AddNodeRequest{
			node.GetPosition()
			});
	}

	for (const auto& segment : solution.segments)
	{
		auto segment1_side = model::ConnectedSegmentSide{};
		auto segment2_side = model::ConnectedSegmentSide{};
		if (segment.m_orientation == model::NetSegmentOrientation::horizontal)
		{
			if (solution.nodes[segment.m_firstNodeId.value].GetPosition().x <= solution.nodes[segment.m_secondNodeId.value].GetPosition().x) // node 0 on left
			{
				segment1_side = model::ConnectedSegmentSide::east;
				segment2_side = model::ConnectedSegmentSide::west;
			}
			else
			{
				segment1_side = model::ConnectedSegmentSide::west;
				segment2_side = model::ConnectedSegmentSide::east;
			}
		}
		if (segment.m_orientation == model::NetSegmentOrientation::vertical)
		{
			if (solution.nodes[segment.m_firstNodeId.value].GetPosition().y <= solution.nodes[segment.m_secondNodeId.value].GetPosition().y) // node 0 on top
			{
				segment1_side = model::ConnectedSegmentSide::south;
				segment2_side = model::ConnectedSegmentSide::north;
			}
			else
			{
				segment1_side = model::ConnectedSegmentSide::north;
				segment2_side = model::ConnectedSegmentSide::south;
			}
		}
		request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
			NetModificationRequest::NodeIdType::new_id,
			NetModificationRequest::NodeIdType::new_id,
			segment1_side,
			segment2_side,
			segment.m_orientation,
			segment.m_firstNodeId,
			segment.m_secondNodeId
			});
	}

	BlockSocketObject* start_socket = nullptr;
	if (std::holds_alternative<SocketAnchor>(m_start_anchor))
	{
		start_socket = std::get<SocketAnchor>(m_start_anchor).socket.GetObjectPtr();
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
	else if (std::holds_alternative<SegmentAnchor>(m_start_anchor))
	{
		auto* start_segment = std::get<SegmentAnchor>(m_start_anchor).segment.GetObjectPtr();
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
		if (std::holds_alternative<SocketAnchor>(m_start_anchor))
		{
			owned_segment = std::get<SocketAnchor>(m_start_anchor).socket.GetObjectPtr();
		}
		if (it != sockets.end() && owned_segment != it->get())
		{
			end_socket = it->get();
			break;
		}
	}
	return end_socket;
}

void node::logic::NewNetLogic::CleanUp()
{
	GraphicsScene* scene = GetScene();
	for (auto&& node : m_nodes)
	{
		if (auto ptr = node.GetObjectPtr())
		{
			scene->PopObject(ptr);
		}
	}
	for (auto&& segment : m_segments)
	{
		if (auto ptr = segment.GetObjectPtr())
		{
			scene->PopObject(ptr);
		}
	}
}

