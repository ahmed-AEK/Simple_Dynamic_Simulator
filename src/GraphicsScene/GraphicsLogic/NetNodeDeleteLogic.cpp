#include "GraphicsLogic/NetNodeDeleteLogic.hpp"
#include "NetObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"
#include "BlockSocketObject.hpp"
#include "BlockObject.hpp"

node::logic::NetNodeDeleteLogic::NetNodeDeleteLogic(NetNode& segment, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_node{segment.GetFocusHandlePtr()}
{
}

void node::logic::NetNodeDeleteLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	const auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_node.isAlive() || new_hover != m_node.GetObjectPtr())
	{
		GetScene()->SetGraphicsLogic(nullptr);
	}
}

MI::ClickEvent node::logic::NetNodeDeleteLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_node.isAlive() || new_hover != m_node.GetObjectPtr())
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	NetModificationRequest request;
	NetNode& node = *static_cast<NetNode*>(m_node.GetObjectPtr());
	assert(node.GetId());
	if (!node.GetId())
	{
		return MI::ClickEvent::CAPTURE_END;
	}
	request.removed_nodes.push_back(*node.GetId());

	if (auto socket = node.GetConnectedSocket())
	{
		assert(socket->GetId());
		assert(socket->GetParentBlock());
		assert(socket->GetParentBlock()->GetModelId());
		if (socket->GetId() && socket->GetParentBlock() && socket->GetParentBlock()->GetModelId())
		{
			request.removed_connections.push_back({ *socket->GetId(), *socket->GetParentBlock()->GetModelId() });
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (auto segment = node.getSegment(static_cast<model::ConnectedSegmentSide>(i)))
		{
			assert(segment->GetId());
			if (auto id = segment->GetId())
			{
				request.removed_segments.push_back(*id);
			}
			
			NetNode* other_connected_node = segment->getStartNode();
			if (other_connected_node == &node)
			{
				other_connected_node = segment->getEndNode();
			}

			if (other_connected_node->GetConnectedSegmentsCount() != 1)
			{
				continue;
			}

			assert(other_connected_node->GetId());
			if (auto id = other_connected_node->GetId())
			{
				request.removed_nodes.push_back(*id);
			}

			if (auto* socket = other_connected_node->GetConnectedSocket())
			{
				assert(socket->GetId());
				assert(socket->GetParentBlock());
				assert(socket->GetParentBlock()->GetModelId());
				if (socket->GetId() && socket->GetParentBlock() && socket->GetParentBlock()->GetModelId())
				{
					request.removed_connections.push_back({ *socket->GetId(), *socket->GetParentBlock()->GetModelId() });
				}
			}
		}
	}

	GetObjectsManager()->GetSceneModel()->UpdateNet(request);
	return MI::ClickEvent::CAPTURE_END;
}



