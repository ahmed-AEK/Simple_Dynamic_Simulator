#include "DeleteTool.hpp"
#include "GraphicsScene.hpp"
#include "BlockObject.hpp"
#include "GraphicsObjectsManager.hpp"

MI::ClickEvent node::DeleteTool::OnLMBDown(const model::Point& p)
{
	GetScene()->ClearCurrentSelection();
	auto* obj = GetScene()->GetObjectAt(p);
	if (!obj)
	{
		return MI::ClickEvent::NONE;
	}
	m_focused_object = obj;
	GetScene()->AddSelection(obj->GetMIHandlePtr());
	return MI::ClickEvent::NONE;
}

MI::ClickEvent node::DeleteTool::OnLMBUp(const model::Point& p)
{
	auto* obj = GetScene()->GetObjectAt(p);
	if (!obj || obj != m_focused_object)
	{
		m_focused_object = nullptr;
		return MI::ClickEvent::NONE;
	}

	if (obj->GetObjectType() == ObjectType::block)
	{
		assert(GetObjectsManager());
		GetObjectsManager()->GetSceneModel()->RemoveBlockById(static_cast<BlockObject*>(obj)->GetModelId());
	}
	return MI::ClickEvent::NONE;
}

void node::DeleteTool::OnMouseMove(const model::Point& p)
{
	if (m_focused_object)
	{
		const auto* obj = GetScene()->GetObjectAt(p);
		if (m_focused_object != obj)
		{
			m_focused_object = nullptr;
		}
	}
}
