#pragma once

#include "toolgui/Scene.hpp"
#include "GraphicsScene/ToolsManager.hpp"

namespace node
{

class Application;
class NodeGraphicsScene;
class GraphicsObjectsManager;

class MainNodeScene: public node::Scene
{
public:
    MainNodeScene(SDL_Rect rect, node::Application* parent);
    NodeGraphicsScene* GetNodeScene() { return m_graphicsScene; }
    ~MainNodeScene() override;
    ToolsManager* GetToolsManager() const { return m_toolsManager.get(); }
protected:

    virtual bool OnRMBUp(const SDL_Point& p) override;
    NodeGraphicsScene* m_graphicsScene = nullptr;
private:
    void InitializeTools();
    void InitializeSidePanel(node::GraphicsScene* gScene);
    std::shared_ptr<ToolsManager> m_toolsManager;
    std::unique_ptr<GraphicsObjectsManager> m_graphicsObjectsManager;

};

}