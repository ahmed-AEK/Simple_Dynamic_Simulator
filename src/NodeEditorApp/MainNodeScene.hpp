#pragma once

#include "toolgui/Scene.hpp"
#include "GraphicsScene/ToolsManager.hpp"
#include "SimulatorRunner.hpp"

namespace node
{

class Application;
class NodeGraphicsScene;
class GraphicsObjectsManager;
class BlockClassesManager;

class MainNodeScene: public node::Scene
{
public:
    MainNodeScene(SDL_Rect rect, node::Application* parent);
    void OnInit() override;
    NodeGraphicsScene* GetNodeScene() { return m_graphicsScene; }
    ~MainNodeScene() override;
    ToolsManager* GetToolsManager() const { return m_toolsManager.get(); }
    void CheckSimulatorEnded();

protected:
    void OnSimulationEnd(SimulationEvent& event);
    virtual bool OnRMBUp(const SDL_Point& p) override;
    NodeGraphicsScene* m_graphicsScene = nullptr;
private:
    void RunSimulator();
    void StopSimulator();
    void InitializeTools();
    void InitializeSidePanel(node::GraphicsScene* gScene);
    std::shared_ptr<ToolsManager> m_toolsManager;
    std::unique_ptr<GraphicsObjectsManager> m_graphicsObjectsManager;
    std::shared_ptr<BlockClassesManager> m_classesManager;

    std::shared_ptr<SimulatorRunner> current_running_simulator = nullptr;
};

}