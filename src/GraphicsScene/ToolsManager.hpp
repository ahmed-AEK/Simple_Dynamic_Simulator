#pragma once

#include <string>
#include <memory>
#include <vector>
#include "NodeModels/Observer.hpp"

namespace node
{
class GraphicsScene;
class GraphicsTool;
class ToolBar;

class ToolsManager
{
public:
	explicit ToolsManager(ToolBar* toolbar);
	~ToolsManager();
	void ChangeTool(const std::string& tool_name);
	void SetTemporaryTool(const std::string& tool_name);
	void RemoveTemporaryTool(const std::string& tool_name);
	void AddTool(std::string name, std::shared_ptr<GraphicsTool> tool);
	const std::shared_ptr<GraphicsTool>& GetCurrentTool() const { return m_current_tool; }
private:
	std::string m_current_tool_name;
	std::string m_current_temporary_tool_name;
	ToolBar* m_toolbar = nullptr;
	std::shared_ptr<GraphicsTool> m_current_tool;
	std::vector<std::pair<std::string, std::shared_ptr<GraphicsTool>>> m_tools;
};

}