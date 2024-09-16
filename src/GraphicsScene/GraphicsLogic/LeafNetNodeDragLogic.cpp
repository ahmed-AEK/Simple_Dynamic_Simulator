#include "LeafNetNodeDragLogic.hpp"
#include "NetObject.hpp"
#include "GraphicsObjectsManager.hpp"
#include "BlockObject.hpp"
#include "BlockSocketObject.hpp"

std::unique_ptr<node::logic::LeafNetNodeDragLogic> node::logic::LeafNetNodeDragLogic::TryCreate(NetNode& node,
	GraphicsScene& scene, GraphicsObjectsManager& manager)
{
	assert(node.GetConnectedSegmentsCount() == 1);
	NetSegment* connected_segment = node.getSegment(model::ConnectedSegmentSide::east);
	if (!connected_segment)
	{
		connected_segment = node.getSegment(model::ConnectedSegmentSide::west);
	}
	if (!connected_segment)
	{
		return nullptr;
	}

	NetNode* connected_node = connected_segment->getStartNode();
	if (connected_node == &node)
	{
		connected_node = connected_segment->getEndNode();
	}
	assert(connected_node);

	return std::make_unique<LeafNetNodeDragLogic>(node, *connected_node, &scene, &manager);
}

node::logic::LeafNetNodeDragLogic::LeafNetNodeDragLogic(NetNode& dragged_node, NetNode& connected_node,
	GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_first_node_start_point{dragged_node.getCenter()}, 
	m_second_node_start_point{connected_node.getCenter()}, 
	m_dragged_node{dragged_node.GetFocusHandlePtr()},
	m_connected_node{connected_node.GetFocusHandlePtr()}
{
}

static node::NetNode* AsNode(const node::HandlePtr<node::GraphicsObject>& obj)
{
	return static_cast<node::NetNode*>(obj.GetObjectPtr());
}

void node::logic::LeafNetNodeDragLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	if (!m_dragged_node.isAlive())
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return;
	}
	assert(m_connected_node.isAlive());

	BlockSocketObject* connectd_socket = GetSocketAt(current_mouse_point);
	model::Point new_point = current_mouse_point;
	if (connectd_socket)
	{
		new_point = connectd_socket->GetCenterInSpace();
	}

	auto* dragged_node = AsNode(m_dragged_node);
	dragged_node->setCenter(new_point);
	auto* connected_node = AsNode(m_connected_node);
	connected_node->setCenter({ m_second_node_start_point.x, new_point.y });
	connected_node->UpdateConnectedSegments();
}

MI::ClickEvent node::logic::LeafNetNodeDragLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	if (!m_dragged_node.isAlive())
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return MI::ClickEvent::NONE;
	}
	assert(m_connected_node.isAlive());

	BlockSocketObject* connectd_socket = GetSocketAt(current_mouse_point);
	model::Point new_point = current_mouse_point;
	if (connectd_socket)
	{
		new_point = connectd_socket->GetCenterInSpace();
	}
	assert(AsNode(m_dragged_node)->GetId());
	assert(AsNode(m_connected_node)->GetId());

	std::optional<model::SocketUniqueId> connected_socket_id;
	if (connectd_socket)
	{
		assert(connectd_socket->GetId());
		assert(connectd_socket->GetParentBlock()->GetModelId());
		connected_socket_id = model::SocketUniqueId{ *connectd_socket->GetId(), 
			*connectd_socket->GetParentBlock()->GetModelId() };
	}


	NetModificationRequest request;

	// request node position change
	request.update_nodes.push_back(NetModificationRequest::UpdateNodeRequest{ *AsNode(m_dragged_node)->GetId(), new_point });
	request.update_nodes.push_back(NetModificationRequest::UpdateNodeRequest{ *AsNode(m_connected_node)->GetId(), {m_second_node_start_point.x,new_point.y }});
	

	// remove old connection
	const auto* old_connected_socket = AsNode(m_dragged_node)->GetConnectedSocket();
	if (old_connected_socket && old_connected_socket != connectd_socket)
	{
		assert(old_connected_socket->GetId());
		assert(old_connected_socket->GetParentBlock()->GetModelId());
		request.removed_connections.push_back(model::SocketUniqueId{ *old_connected_socket->GetId(), *old_connected_socket->GetParentBlock()->GetModelId() });
	}

	// establish new connection
	if (connected_socket_id && old_connected_socket != connectd_socket)
	{
		request.added_connections.push_back(NetModificationRequest::SocketConnectionRequest{
			*connected_socket_id, NetModificationRequest::NodeIdType::existing_id, *AsNode(m_dragged_node)->GetId()
			});
	}
	CleanUp();
	GetObjectsManager()->GetSceneModel()->UpdateNet(request);
	return MI::ClickEvent::CLICKED;
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
				auto socket_rect = ToSDLRect(socket->GetSpaceRect());
				if (SDL_PointInRect(&current_mouse_point_SDL, &socket_rect))
				{
					return true;
				}
				return false;
			});
		if (it != sockets.end())
		{
			auto* connected_node = it->get()->GetConnectedNode();
			if (connected_node == nullptr || connected_node == m_dragged_node.GetObjectPtr())
			{
				end_socket = it->get();
				break;
			}
		}
	}
	return end_socket;
}

void node::logic::LeafNetNodeDragLogic::CleanUp()
{
	if (!m_dragged_node.isAlive())
	{
		return;
	}
	assert(m_connected_node.isAlive());

	auto* dragged_node = AsNode(m_dragged_node);
	dragged_node->setCenter(m_first_node_start_point);
	auto* connected_node = AsNode(m_connected_node);
	connected_node->setCenter(m_second_node_start_point);
	connected_node->UpdateConnectedSegments();
}

