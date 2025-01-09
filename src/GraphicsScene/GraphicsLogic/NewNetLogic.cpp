#include "GraphicsLogic/NewNetLogic.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"
#include <array>
#include <algorithm>

std::unique_ptr<node::logic::NewNetLogic> node::logic::NewNetLogic::CreateFromSocket(BlockSocketObject& socket, 
	GraphicsScene* scene, GraphicsObjectsManager* manager)
{
	assert(scene);
	std::array<NetNode*, 4> nodes{};
	std::array<NetSegment*, 3> segments{};
	try
	{
		int layer = 1000;
		for (auto&& segment : segments)
		{
			auto new_segmet = std::make_unique<NetSegment>(model::NetSegmentOrientation::vertical, nullptr, nullptr);
			segment = new_segmet.get();
			scene->AddObject(std::move(new_segmet), layer);
			layer++;
		}
		for (auto&& node : nodes)
		{
			auto new_node = std::make_unique<NetNode>(model::Point{ 0,0 });
			node = new_node.get();
			scene->AddObject(std::move(new_node), layer);
			layer++;
		}
		assert(manager);
		return std::make_unique<NewNetLogic>(SocketAnchor{socket.GetFocusHandlePtr(), socket.GetCenterInSpace()}, nodes, segments, scene, manager);
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
	std::array<NetNode*, 4> nodes{};
	std::array<NetSegment*, 3> segments{};
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
		return std::make_unique<NewNetLogic>(SegmentAnchor{ base_segment.GetFocusHandlePtr(), {base_segment.getStartNode()->getCenter().x, start_point.y}}, nodes, segments, scene, manager);
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

node::logic::NewNetLogic::NewNetLogic(anchor_t start_anchor, std::array<NetNode*, 4> nodes,
	std::array<NetSegment*, 3> segments, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager }, m_start_anchor{std::move(start_anchor)}
{
	
	std::transform(nodes.begin(), nodes.end(), m_nodes.begin(), 
		[](const auto& node)
		{
			assert(node);
			return node->GetFocusHandlePtr();
		});
	std::transform(segments.begin(), segments.end(), m_segments.begin(),
		[](const auto& segment)
		{
			assert(segment);
			return segment->GetFocusHandlePtr();
		});
	for (auto&& node : nodes)
	{
		node->setCenter(std::visit([](const auto& anchor) {return anchor.position; }, m_start_anchor));
	}
	segments[0]->Connect(nodes[0], nodes[1], model::NetSegmentOrientation::horizontal);
	segments[1]->Connect(nodes[1], nodes[2], model::NetSegmentOrientation::vertical);
	segments[2]->Connect(nodes[2], nodes[3], model::NetSegmentOrientation::horizontal);
}

static node::NetNode* AsNode(const node::HandlePtr<node::GraphicsObject>& obj)
{
	assert(obj.isAlive());
	return static_cast<node::NetNode*>(obj.GetObjectPtr());
}

static node::NetSegment* AsSegment(const node::HandlePtr<node::GraphicsObject>& obj)
{
	assert(obj.isAlive());
	return static_cast<node::NetSegment*>(obj.GetObjectPtr());
}

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
	model::node_int midpoint_x = (end_point.x + start.x) / 2;
	AsNode(m_nodes[1])->setCenter({ midpoint_x, start.y });
	AsNode(m_nodes[2])->setCenter({ midpoint_x, end_point.y });
	AsNode(m_nodes[3])->setCenter({ end_point.x, end_point.y });
	AsSegment(m_segments[0])->CalcRect();
	AsSegment(m_segments[1])->CalcRect();
	AsSegment(m_segments[2])->CalcRect();
}

MI::ClickEvent node::logic::NewNetLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	if (!std::visit(AnchorAlive{}, m_start_anchor))
	{
		DeleteAllOwnedObjects();
		return MI::ClickEvent::CAPTURE_END;
	}

	assert(GetObjectsManager());
	auto request = PopulateResultNet(current_mouse_point);
	DeleteAllOwnedObjects();

	GetObjectsManager()->GetSceneModel()->UpdateNet(request);
	return MI::ClickEvent::CAPTURE_END;
}

void node::logic::NewNetLogic::OnCancel()
{
	DeleteAllOwnedObjects();
}

node::NetModificationRequest node::logic::NewNetLogic::PopulateResultNet(const model::Point& current_mouse_point)
{
	using model::NetNodeId;
	using model::NetSegmentId;
	using enum model::ConnectedSegmentSide;

	const auto* end_socket = GetSocketAt(current_mouse_point);

	NetModificationRequest request;
	
	request.added_nodes.push_back(NetModificationRequest::AddNodeRequest{
		AsNode(m_nodes[0])->getCenter()
		});
	request.added_nodes.push_back(NetModificationRequest::AddNodeRequest{
		AsNode(m_nodes[1])->getCenter()
		});
	request.added_nodes.push_back(NetModificationRequest::AddNodeRequest{
		AsNode(m_nodes[2])->getCenter()
		});
	request.added_nodes.push_back(NetModificationRequest::AddNodeRequest{
		AsNode(m_nodes[3])->getCenter()
		});

	{
		auto segment1_side = model::ConnectedSegmentSide{};
		auto segment2_side = model::ConnectedSegmentSide{};
		if (AsNode(m_nodes[0])->getCenter().x < AsNode(m_nodes[1])->getCenter().x) // node 0 on left
		{
			segment1_side = model::ConnectedSegmentSide::east;
			segment2_side = model::ConnectedSegmentSide::west;
		}
		else
		{
			segment1_side = model::ConnectedSegmentSide::west;
			segment2_side = model::ConnectedSegmentSide::east;
		}
		request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
			NetModificationRequest::NodeIdType::new_id,
			NetModificationRequest::NodeIdType::new_id,
			segment1_side,
			segment2_side,
			model::NetSegmentOrientation::horizontal,
			NetNodeId{0},
			NetNodeId{1}
		});
	}

	{
		auto segment1_side = model::ConnectedSegmentSide{};
		auto segment2_side = model::ConnectedSegmentSide{};
		if (AsNode(m_nodes[1])->getCenter().y <= AsNode(m_nodes[2])->getCenter().y) // node1 above node 2
		{
			segment1_side = model::ConnectedSegmentSide::south;
			segment2_side = model::ConnectedSegmentSide::north;
		}
		else
		{
			segment1_side = model::ConnectedSegmentSide::north;
			segment2_side = model::ConnectedSegmentSide::south;
		}
		request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
			NetModificationRequest::NodeIdType::new_id,
			NetModificationRequest::NodeIdType::new_id,
			segment1_side,
			segment2_side,
			model::NetSegmentOrientation::vertical,
			NetNodeId{1},
			NetNodeId{2}
			});
	}

	{
		auto segment1_side = model::ConnectedSegmentSide{};
		auto segment2_side = model::ConnectedSegmentSide{};
		if (AsNode(m_nodes[2])->getCenter().x < AsNode(m_nodes[3])->getCenter().x) // node 2 on left
		{
			segment1_side = model::ConnectedSegmentSide::east;
			segment2_side = model::ConnectedSegmentSide::west;
		}
		else
		{
			segment1_side = model::ConnectedSegmentSide::west;
			segment2_side = model::ConnectedSegmentSide::east;
		}
		request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
			NetModificationRequest::NodeIdType::new_id,
			NetModificationRequest::NodeIdType::new_id,
			segment1_side,
			segment2_side,
			model::NetSegmentOrientation::horizontal,
			NetNodeId{2},
			NetNodeId{3}
			});
	}

	BlockSocketObject* start_socket = nullptr;
	if (std::holds_alternative<SocketAnchor>(m_start_anchor))
	{
		start_socket = static_cast<BlockSocketObject*>(std::get<SocketAnchor>(m_start_anchor).socket.GetObjectPtr());
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
		auto* start_segment = static_cast<NetSegment*>(std::get<SegmentAnchor>(m_start_anchor).segment.GetObjectPtr());
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
			NetNodeId{3}
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

void node::logic::NewNetLogic::DeleteAllOwnedObjects()
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

