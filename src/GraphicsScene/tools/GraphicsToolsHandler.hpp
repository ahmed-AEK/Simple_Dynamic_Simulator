#pragma once

#include "GraphicsScene/GraphicsScene.hpp"

namespace node
{

class ToolsManager;
class HoverHighlightObject;

class GraphicsToolsHandler : public ToolHandler
{
public:
    GraphicsToolsHandler(GraphicsScene& scene, std::weak_ptr<GraphicsObjectsManager> objects_manager, std::weak_ptr<ToolsManager> tools_manager);
    bool IsCapturingMouse() const override;
    void OnMouseMove(GraphicsTool::MouseHoverEvent& e) override;
    MI::ClickEvent OnLMBDown(GraphicsTool::MouseButtonEvent& e) override;
    MI::ClickEvent OnLMBUp(GraphicsTool::MouseButtonEvent& e) override;
private:

    std::weak_ptr<ToolsManager> m_tools_manager;
    std::weak_ptr<GraphicsObjectsManager> m_objects_manager;
    GraphicsScene* m_scene;
    HandlePtrS<HoverHighlightObject, GraphicsObject> m_hover_highlight_object;
};
}