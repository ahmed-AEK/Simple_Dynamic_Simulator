#include "DeleteTool.hpp"
#include "GraphicsScene.hpp"
#include "BlockObject.hpp"
#include "NetObject.hpp"
#include "GraphicsObjectsManager.hpp"
#include "GraphicsLogic/BlockDeleteLogic.hpp"
#include "GraphicsLogic/SegmentDeleteLogic.hpp"
#include "GraphicsLogic/NetNodeDeleteLogic.hpp"

MI::ClickEvent node::DeleteTool::OnLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager)
{
	scene.ClearCurrentSelection();
	auto* obj = scene.GetObjectAt(e.point);
	if (!obj)
	{
		return MI::ClickEvent::NONE;
	}
	switch (obj->GetObjectType())
	{
	case ObjectType::block:
	{
		scene.SetGraphicsLogic(std::make_unique<logic::BlockDeleteLogic>(*static_cast<BlockObject*>(obj), &scene, &manager));
		scene.AddSelection(obj->GetMIHandlePtr());
		break;
	}
	case ObjectType::netSegment:
	{
		scene.SetGraphicsLogic(std::make_unique<logic::SegmentDeleteLogic>(*static_cast<NetSegment*>(obj), &scene, &manager));
		scene.AddSelection(obj->GetMIHandlePtr());
		break;
	}
	case ObjectType::netNode:
		scene.SetGraphicsLogic(std::make_unique<logic::NetNodeDeleteLogic>(*static_cast<NetNode*>(obj), &scene, &manager));
		scene.AddSelection(obj->GetMIHandlePtr());
		break;
	default: break;
	}
	return MI::ClickEvent::CLICKED;
}

