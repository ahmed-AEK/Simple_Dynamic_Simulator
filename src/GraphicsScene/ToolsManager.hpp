#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace node
{
class GraphicsScene;
class GraphicsTool;

class ToolsManager
{
public:
	ToolsManager(GraphicsScene* scene);
	~ToolsManager();
	void OnChangeTool(const std::string& tool_name);
	void AddTool(std::string name, std::shared_ptr<GraphicsTool> tool);
private:
	std::string m_current_tool;
	GraphicsScene* m_scene = nullptr;
	std::vector<std::pair<std::string,std::shared_ptr<GraphicsTool>>> m_tools;
};

}