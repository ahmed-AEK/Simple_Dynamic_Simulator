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
	for (auto&& object : GetScene()->GetCurrentSelection())
	{
		auto* object_ptr = object.GetObjectPtr();
		if (!object_ptr)
		{
			continue;
		}
		if (object_ptr->GetObjectType() == ObjectType::netSegment)
		{
			auto id_opt = static_cast<NetSegment*>(object_ptr)->GetId();
			assert(id_opt);
			if (id_opt)
			{
				request.removed_segments.push_back(*id_opt);
			}
		}
		if (object_ptr->GetObjectType() == ObjectType::netNode)
		{
			auto* netnode = static_cast<NetNode*>(object_ptr);
			auto id_opt = netnode->GetId();
			assert(id_opt);
			if (id_opt)
			{
				request.removed_nodes.push_back(*id_opt);
			}
			if (netnode->GetConnectedSocket())
			{
				auto block_id_opt = netnode->GetConnectedSocket()->GetUniqueId();
				assert(block_id_opt);
				if (block_id_opt)
				{
					request.removed_connections.push_back(*block_id_opt);
				}
			}
		}
	}
	
	GetObjectsManager()->GetSceneModel()->UpdateNet(request);
	return MI::ClickEvent::CAPTURE_END;
}



