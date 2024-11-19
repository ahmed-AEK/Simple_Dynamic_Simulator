#pragma once

#include "GraphicsScene/GraphicsScene.hpp"

namespace node
{

class ToolsManager;

class GraphicsToolHandler : public ToolHandler
{
public:
    GraphicsToolHandler(GraphicsScene& scene, std::weak_ptr<GraphicsObjectsManager> objects_manager, std::weak_ptr<ToolsManager> tools_manager);
    virtual bool IsCapturingMouse() const;
    virtual void OnMouseMove(GraphicsTool::MouseHoverEvent& e);
    virtual MI::ClickEvent OnLMBDown(GraphicsTool::MouseButtonEvent& e);
    virtual MI::ClickEvent OnLMBUp(GraphicsTool::MouseButtonEvent& e);
private:
    std::weak_ptr<ToolsManager> m_tools_manager;
    std::weak_ptr<GraphicsObjectsManager> m_objects_manager;
    GraphicsScene* m_scene;
};
}