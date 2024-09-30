#pragma once

#include "toolgui/Scene.hpp"
#include "GraphicsScene/ToolsManager.hpp"
#include "SimulatorRunner.hpp"
#include "NodeEditorApp/SimulationManager.hpp"

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
    explicit NodeSceneEventReceiver(MainNodeScene& scene) : m_scene{ &scene } {}
    virtual ~NodeSceneEventReceiver() = default;
    MainNodeScene* GetScene() { return m_scene; }
private:
    MainNodeScene* m_scene;
};

enum class DialogType
{
    BlockDialog,
    PropertiesDialog,
};

struct DialogSlot
{
    HandlePtr<Widget> dialog;
    DialogType type;
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

    void DeleteEventReceiver(const NodeSceneEventReceiver* handler);
    void AddEventReceiver(std::unique_ptr<NodeSceneEventReceiver> handler) { m_event_receivers.push_back(std::move(handler)); }
protected:
    virtual bool OnRMBUp(const SDL_Point& p) override;
    NodeGraphicsScene* m_graphicsScene = nullptr;
private:
    void RunSimulator();
    void OnSimulationEnd(const SimulationEvent& event);
    
    void OnSettingsClicked();

    void InitializeTools();
    void InitializeSidePanel(node::GraphicsScene* gScene);
    
    void OpenPropertiesDialog();
    void OpenPropertiesDialog(BlockObject& object);
    void OpenBlockDialog(BlockObject& block);

    void OnUndo();
    void OnRedo();
    bool CanUndo();
    bool CanRedo();

    std::shared_ptr<ToolsManager> m_toolsManager;
    std::shared_ptr<GraphicsObjectsManager> m_graphicsObjectsManager;
    std::shared_ptr<BlockClassesManager> m_classesManager;
    std::shared_ptr<BlockStylerFactory> m_blockStylerFactory;
    std::vector<std::unique_ptr<NodeSceneEventReceiver>> m_event_receivers;
    std::unordered_map<BlockObject*, DialogSlot> m_objects_dialogs;

    SimulationManager m_sim_mgr;

    HandlePtr<Widget> m_settings_dialog;

};

}