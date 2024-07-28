#pragma once

#include <string>
#include <memory>
#include <vector>

namespace node
{
class GraphicsScene;
class GraphicsTool;
class ToolBar;

class ToolsManager
{
public:
	ToolsManager(GraphicsScene* scene, ToolBar* toolbar);
	~ToolsManager();
	void OnChangeTool(const std::string& tool_name);
	void AddTool(std::string name, std::shared_ptr<GraphicsTool> tool);
private:
	std::string m_current_tool;
	GraphicsScene* m_scene = nullptr;
	ToolBar* m_toolbar = nullptr;
	std::vector<std::pair<std::string,std::shared_ptr<GraphicsTool>>> m_tools;
};

}