#include "DeleteTool.hpp"
#include "GraphicsScene.hpp"
#include "BlockObject.hpp"
#include "NetObject.hpp"
#include "GraphicsObjectsManager.hpp"
#include "GraphicsLogic/BlockDeleteLogic.hpp"
#include "GraphicsLogic/NetDeleteLogic.hpp"

MI::ClickEvent node::DeleteTool::OnObjectLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager, GraphicsObject& object)
{
	UNUSED_PARAM(e);
	scene.ClearCurrentSelection();

	switch (object.GetObjectType())
	{
	case ObjectType::block:
	{
		scene.SetGraphicsLogic(std::make_unique<logic::BlockDeleteLogic>(*static_cast<BlockObject*>(&object), &scene, &manager));
		scene.AddSelection(object.GetMIHandlePtr());
		break;
	}
	case ObjectType::netSegment:
	{
		scene.SetGraphicsLogic(std::make_unique<logic::NetDeleteLogic>(object, &scene, &manager));
		AddSelectConnectedNet(*static_cast<NetSegment*>(&object), scene);
		break;
	}
	case ObjectType::netNode:
		scene.SetGraphicsLogic(std::make_unique<logic::NetDeleteLogic>(object, &scene, &manager));
		AddSelectConnectedNet(*static_cast<NetNode*>(&object), scene);
		break;
	default: break;
	}
	return MI::ClickEvent::CLICKED;
}

bool node::DeleteTool::IsObjectClickable(GraphicsScene& scene, GraphicsObjectsManager& manager, GraphicsObject& object)
{
	UNUSED_PARAM(scene); UNUSED_PARAM(manager);
	switch (object.GetObjectType())
	{
	case ObjectType::block:
	{
		return true;
	}
	case ObjectType::netSegment:
	{
		return true;
	}
	case ObjectType::netNode:
	{
		return true;
	}
	default: break;
	}
	return false;
}