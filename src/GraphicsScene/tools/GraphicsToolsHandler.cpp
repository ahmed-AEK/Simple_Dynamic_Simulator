#include "GraphicsToolsHandler.hpp"
#include "GraphicsScene/ToolsManager.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"
#include "GraphicsScene/HoverHighlightObject.hpp"

node::GraphicsToolsHandler::GraphicsToolsHandler(GraphicsScene& scene, std::weak_ptr<GraphicsObjectsManager> objects_manager, std::weak_ptr<ToolsManager> tools_manager)
	:m_tools_manager{std::move(tools_manager)}, m_objects_manager{std::move(objects_manager)}, m_scene{&scene}
{
}

bool node::GraphicsToolsHandler::IsCapturingMouse() const
{
	auto tools_manager = m_tools_manager.lock();
	if (!tools_manager)
	{
		return false;
	}
	auto&& ptr = tools_manager->GetCurrentTool();
	if (!ptr)
	{
		return false;
	}

	return ptr->IsCapturingMouse();
}

void node::GraphicsToolsHandler::OnMouseMove(GraphicsTool::MouseHoverEvent& e)
{
	UNUSED_PARAM(e);
	auto tools_manager = m_tools_manager.lock();
	if (!tools_manager)
	{
		return;
	}
	auto tool = tools_manager->GetCurrentTool();
	if (!tool)
	{
		return;
	}

	auto objects_manager = m_objects_manager.lock();
	if (!objects_manager)
	{
		return;
	}

	auto* obj = m_scene->GetCurrentHover();
	if (!obj)
	{
		if (auto* hover_highlight_object = m_hover_highlight_object.GetObjectPtr())
		{
			if (auto* hovered = hover_highlight_object->GetAttachedToObject())
			{
				hovered->SetAttachment(nullptr);
			}
		}
		return;
	}
	if (!tool->IsObjectClickable(*m_scene, *objects_manager, *obj) || 
		(obj->GetAttachment() != m_hover_highlight_object.GetObjectPtr() &&
		obj->GetAttachment() != nullptr))
	{
		if (auto* hover_highlight_object = m_hover_highlight_object.GetObjectPtr())
		{
			if (auto* hovered = hover_highlight_object->GetAttachedToObject())
			{
				hovered->SetAttachment(nullptr);
			}
		}
		return;
	}

	if (!obj->GetAttachment())
	{
		auto* hover_highlight_object = m_hover_highlight_object.GetObjectPtr();
		if (!hover_highlight_object)
		{
			auto object_rect = HoverHighlightObject::RectForObject(obj->GetSceneRect());
			auto new_hover_object = std::make_unique<HoverHighlightObject>(*obj, objects_manager.get(), model::ObjectSize{object_rect.w, object_rect.h});
			m_hover_highlight_object.reset(*new_hover_object);
			hover_highlight_object = new_hover_object.get();
			m_scene->AddObject(std::move(new_hover_object), GraphicsScene::InteractiveLayer);
		}
		obj->SetAttachment(hover_highlight_object);
		return;
	}
}

MI::ClickEvent node::GraphicsToolsHandler::OnLMBDown(GraphicsTool::MouseButtonEvent& e)
{
	auto tools_manager = m_tools_manager.lock();
	if (!tools_manager)
	{
		return MI::ClickEvent::CLICKED;
	}
	auto tool = tools_manager->GetCurrentTool();
	if (!tool)
	{
		return MI::ClickEvent::CLICKED;
	}

	auto objects_manager = m_objects_manager.lock();
	if (!objects_manager)
	{
		return MI::ClickEvent::CLICKED;
	}

	auto* obj = m_scene->GetCurrentHover();
	if (auto* hover_highlight_object = m_hover_highlight_object.GetObjectPtr())
	{
		if (auto* attached_object = hover_highlight_object->GetAttachedToObject())
		{
			attached_object->SetAttachment(nullptr);
		}
	}
	if (!obj)
	{
		return tool->OnSpaceLMBDown(e, *m_scene, *objects_manager);
	}
	if (tool->IsObjectClickable(*m_scene, *objects_manager, *obj))
	{
		return tool->OnObjectLMBDown(e, *m_scene, *objects_manager, *obj);
	}
	return MI::ClickEvent::NONE;
}

MI::ClickEvent node::GraphicsToolsHandler::OnLMBUp(GraphicsTool::MouseButtonEvent& e)
{
	UNUSED_PARAM(e);
	auto tools_manager = m_tools_manager.lock();
	if (!tools_manager)
	{
		return MI::ClickEvent::CLICKED;
	}
	auto tool = tools_manager->GetCurrentTool();
	if (!tool)
	{
		return MI::ClickEvent::CLICKED;
	}

	auto objects_manager = m_objects_manager.lock();
	if (!objects_manager)
	{
		return MI::ClickEvent::CLICKED;
	}
	return MI::ClickEvent::NONE;
}



