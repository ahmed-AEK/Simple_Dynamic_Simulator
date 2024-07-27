#include "ToolsManager.hpp"
#include "SDL_Framework/SDL_Headers.h"
#include "GraphicsScene/GraphicsScene.hpp"
#include "tools/GraphicsTool.hpp"
#include "toolgui/ToolBar.hpp"

static auto GetToolByName(const std::string& tool_name, auto&& vec) -> decltype(vec.begin())
{
	auto it = std::find_if(vec.begin(), vec.end(), [&](auto&& value) {return value.first == tool_name; });
	return it;
}

void node::ToolsManager::OnChangeTool(const std::string& tool_name)
{
	auto it = GetToolByName(tool_name, m_tools);
	if (it != m_tools.end())
	{
		auto btn1 = m_toolbar->GetButton(m_current_tool);
		if (btn1)
		{
			btn1->SetActive(false);
		}

		m_current_tool = tool_name;
		SDL_Log("changed tool to %s", m_current_tool.c_str());
		m_scene->SetTool(it->second);
		auto btn2 = m_toolbar->GetButton(tool_name);
		if (btn2)
		{
			btn2->SetActive(true);
		}
	}
	else
	{
		SDL_LogError(0, "failed to change tool to %s", tool_name.c_str());
	}
}

node::ToolsManager::ToolsManager(GraphicsScene* scene, ToolBar* toolbar)
	:m_scene{scene}, m_toolbar{toolbar}
{

}

node::ToolsManager::~ToolsManager()
{

}


void node::ToolsManager::AddTool(std::string name, std::shared_ptr<node::GraphicsTool> tool)
{
	assert(GetToolByName(name, m_tools) == m_tools.end());
	m_tools.push_back({ name, std::move(tool) });
}