#include "NetDeleteLogic.hpp"
#include "BlockDeleteLogic.hpp"
#include "NetObject.hpp"
#include "BlockObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"
#include "NetUtils/Utils.hpp"

#include "GraphicsScene/BlockSocketObject.hpp"
#include "NodeModels/SceneModelManager.hpp"
#include "NetUtils/DeleteHelpers.hpp"

#include <set>

node::logic::NetDeleteLogic::NetDeleteLogic(GraphicsObject& object, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_object{ object.GetMIHandlePtr() }
{
}

void node::logic::NetDeleteLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	const auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_object.isAlive() || new_hover != m_object.GetObjectPtr())
	{
		GetScene()->SetGraphicsLogic(nullptr);
	}
}

MI::ClickEvent node::logic::NetDeleteLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	const auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_object.isAlive() || new_hover != m_object.GetObjectPtr())
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	auto scene = GetObjectsManager()->GetSceneModel();
	auto request = NetUtils::GetDeletionRequestForNet(GetScene()->GetCurrentSelection(), scene->GetModel());
	if (!request)
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	GetObjectsManager()->GetSceneModel()->UpdateNet(std::move(*request));
	return MI::ClickEvent::CAPTURE_END;
}


std::optional<node::NetModificationRequest> node::NetUtils::GetDeletionRequestForNet(
	std::span<node::HandlePtr<node::GraphicsObject>> objects, const model::NodeSceneModel& scene)
{
	using namespace node;
	if (objects.size() == 0)
	{
		return {};
	}

	std::vector<model::NetSegmentId> segments_to_remove;
	std::vector<model::NetNodeId> nodes_to_remove;
	for (auto&& object : objects)
	{
		auto* object_ptr = object.GetObjectPtr();
		if (!object_ptr)
		{
			continue;
		}
		if (object_ptr->GetObjectType() == ObjectType::netSegment)
		{
			auto* net_segment = static_cast<NetSegment*>(object_ptr);
			auto id_opt = net_segment->GetId();
			assert(id_opt);
			if (id_opt)
			{
				segments_to_remove.push_back(*id_opt);
			}
		}
		if (object_ptr->GetObjectType() == ObjectType::netNode)
		{
			auto* netnode = static_cast<NetNode*>(object_ptr);
			auto id_opt = netnode->GetId();
			assert(id_opt);
			if (id_opt)
			{
				nodes_to_remove.push_back(*id_opt);
			}
		}
	}

	return GetDeletionRequestForNet(segments_to_remove, nodes_to_remove, scene);
}