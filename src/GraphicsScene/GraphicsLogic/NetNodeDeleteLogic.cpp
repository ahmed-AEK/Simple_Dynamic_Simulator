#include "GraphicsLogic/NetNodeDeleteLogic.hpp"
#include "NetObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"
#include "BlockSocketObject.hpp"
#include "BlockObject.hpp"

node::logic::NetNodeDeleteLogic::NetNodeDeleteLogic(NetNode& segment, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_node{segment}
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
	NetNode& node = *m_node.GetObjectPtr();
	assert(node.GetId());
	if (!node.GetId())
	{
		return MI::ClickEvent::CAPTURE_END;
	}
	for (auto object : GetScene()->GetCurrentSelection())
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
	
	GetObjectsManager()->GetSceneModel()->UpdateNet(std::move(request));
	return MI::ClickEvent::CAPTURE_END;
}



