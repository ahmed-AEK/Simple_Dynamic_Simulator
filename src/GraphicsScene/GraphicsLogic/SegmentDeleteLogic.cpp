#include "SegmentDeleteLogic.hpp"
#include "NetObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"
#include "BlockSocketObject.hpp"
#include "BlockObject.hpp"

node::logic::SegmentDeleteLogic::SegmentDeleteLogic(NetSegment& segment, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_segment{segment}
{
}

void node::logic::SegmentDeleteLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	const auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_segment.isAlive() || new_hover != m_segment.GetObjectPtr())
	{
		GetScene()->SetGraphicsLogic(nullptr);
	}
}

MI::ClickEvent node::logic::SegmentDeleteLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_segment.isAlive() || new_hover != m_segment.GetObjectPtr())
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	NetModificationRequest request;
	NetSegment& segment = *m_segment.GetObjectPtr();
	assert(segment.GetId());
	if (!segment.GetId())
	{
		return MI::ClickEvent::CAPTURE_END;
	}
	request.removed_segments.push_back(*segment.GetId());
	auto add_connected_node = [&](NetNode* node)
		{
			if (node->GetConnectedSegmentsCount() == 1)
			{
				request.removed_nodes.push_back(*node->GetId());
				if (node->GetConnectedSocket())
				{
					assert(node->GetConnectedSocket()->GetId());
					assert(node->GetConnectedSocket()->GetParentBlock()->GetModelId());
					request.removed_connections.push_back(model::SocketUniqueId{ *node->GetConnectedSocket()->GetId(),
						*node->GetConnectedSocket()->GetParentBlock()->GetModelId() });
				}
			}
		};
	add_connected_node(segment.getStartNode());
	add_connected_node(segment.getEndNode());
	
	GetObjectsManager()->GetSceneModel()->UpdateNet(request);
	return MI::ClickEvent::CAPTURE_END;
}



