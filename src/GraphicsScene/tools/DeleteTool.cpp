#include "DeleteTool.hpp"
#include "GraphicsScene.hpp"
#include "BlockObject.hpp"
#include "NetObject.hpp"
#include "GraphicsObjectsManager.hpp"
#include "GraphicsLogic/BlockDeleteLogic.hpp"
#include "GraphicsLogic/SegmentDeleteLogic.hpp"
#include "GraphicsLogic/NetNodeDeleteLogic.hpp"

MI::ClickEvent node::DeleteTool::OnLMBDown(MouseButtonEvent& e)
{
	GetScene()->ClearCurrentSelection();
	auto* obj = GetScene()->GetObjectAt(e.point);
	if (!obj)
	{
		return MI::ClickEvent::NONE;
	}
	switch (obj->GetObjectType())
	{
	case ObjectType::block:
	{
		GetScene()->SetGraphicsLogic(std::make_unique<logic::BlockDeleteLogic>(*static_cast<BlockObject*>(obj), GetScene(), GetObjectsManager()));
		GetScene()->AddSelection(obj->GetMIHandlePtr());
		break;
	}
	case ObjectType::netSegment:
	{
		GetScene()->SetGraphicsLogic(std::make_unique<logic::SegmentDeleteLogic>(*static_cast<NetSegment*>(obj), GetScene(), GetObjectsManager()));
		GetScene()->AddSelection(obj->GetMIHandlePtr());
		break;
	}
	case ObjectType::netNode:
		GetScene()->SetGraphicsLogic(std::make_unique<logic::NetNodeDeleteLogic>(*static_cast<NetNode*>(obj), GetScene(), GetObjectsManager()));
		GetScene()->AddSelection(obj->GetMIHandlePtr());
		break;
	default: break;
	}
	return MI::ClickEvent::CLICKED;
}

MI::ClickEvent node::DeleteTool::OnLMBUp(MouseButtonEvent& e)
{
	UNUSED_PARAM(e);
	return MI::ClickEvent::NONE;
}
