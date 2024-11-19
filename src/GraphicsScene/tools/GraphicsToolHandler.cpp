#include "GraphicsToolHandler.hpp"
#include "GraphicsScene/ToolsManager.hpp"

node::GraphicsToolHandler::GraphicsToolHandler(GraphicsScene& scene, std::weak_ptr<GraphicsObjectsManager> objects_manager, std::weak_ptr<ToolsManager> tools_manager)
	:m_tools_manager{std::move(tools_manager)}, m_objects_manager{std::move(objects_manager)}, m_scene{&scene}
{
}

bool node::GraphicsToolHandler::IsCapturingMouse() const
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

void node::GraphicsToolHandler::OnMouseMove(GraphicsTool::MouseHoverEvent& e)
{
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

	tool->OnMouseMove(e, *m_scene, *objects_manager);
}

MI::ClickEvent node::GraphicsToolHandler::OnLMBDown(GraphicsTool::MouseButtonEvent& e)
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

	return tool->OnLMBDown(e, *m_scene, *objects_manager);
}

MI::ClickEvent node::GraphicsToolHandler::OnLMBUp(GraphicsTool::MouseButtonEvent& e)
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

	return tool->OnLMBUp(e, *m_scene, *objects_manager);
}



