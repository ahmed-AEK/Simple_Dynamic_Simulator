#pragma once

#include "toolgui/Scene.hpp"
#include "GraphicsScene/ToolsManager.hpp"
#include "SimulatorRunner.hpp"

namespace node
{
class MainNodeScene;
class Application;
class NodeGraphicsScene;
class GraphicsObjectsManager;
class BlockClassesManager;
class BlockObject;
class BlockStylerFactory;

class NodeSceneEventReceiver
{
public:
    NodeSceneEventReceiver(MainNodeScene& scene) : m_scene{ &scene } {}
    virtual ~NodeSceneEventReceiver() = default;
    MainNodeScene* GetScene() { return m_scene; }
private:
    MainNodeScene* m_scene;
};

class MainNodeScene: public node::Scene
{
public:
    MainNodeScene(SDL_Rect rect, node::Application* parent);
    void OnInit() override;
    NodeGraphicsScene* GetNodeScene() { return m_graphicsScene; }
    ~MainNodeScene() override;
    ToolsManager* GetToolsManager() const { return m_toolsManager.get(); }
    void CheckSimulatorEnded();

    void DeleteEventReceiver(NodeSceneEventReceiver* handler);
    void AddEventReceiver(std::unique_ptr<NodeSceneEventReceiver> handler) { m_event_receivers.push_back(std::move(handler)); }
protected:
    void OnSimulationEnd(SimulationEvent& event);
    virtual bool OnRMBUp(const SDL_Point& p) override;
    NodeGraphicsScene* m_graphicsScene = nullptr;
private:
    void RunSimulator();
    void StopSimulator();
    void InitializeTools();
    void InitializeSidePanel(node::GraphicsScene* gScene);
    void OpenPropertiesDialog();
    void OpenBlockDialog(BlockObject& block);

    std::shared_ptr<ToolsManager> m_toolsManager;
    std::unique_ptr<GraphicsObjectsManager> m_graphicsObjectsManager;
    std::shared_ptr<BlockClassesManager> m_classesManager;
    std::shared_ptr<BlockStylerFactory> m_blockStylerFactory;
    std::vector<std::unique_ptr<NodeSceneEventReceiver>> m_event_receivers;
    std::unordered_map<BlockObject*, HandlePtr<Widget>> m_objects_dialogs;

    std::shared_ptr<SimulatorRunner> m_current_running_simulator = nullptr;
    std::vector<BlockResult> m_last_simulation_result;
};

}