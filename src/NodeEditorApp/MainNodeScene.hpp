#pragma once

#include "toolgui/Scene.hpp"
#include "GraphicsScene/ToolsManager.hpp"

namespace node
{

class Application;
class NodeGraphicsScene;

class MainNodeScene: public node::Scene
{
public:
    MainNodeScene(SDL_Rect rect, node::Application* parent);
    NodeGraphicsScene* GetNodeScene() { return m_graphicsScene; }
    ~MainNodeScene() override;
protected:

    virtual bool OnRMBUp(const SDL_Point& p) override;
    NodeGraphicsScene* m_graphicsScene = nullptr;
private:
    void InitializeTools(node::GraphicsScene* gScene);
    void InitializeSidePanel(node::GraphicsScene* gScene);
    std::shared_ptr<ToolsManager> m_toolsManager;

};

}