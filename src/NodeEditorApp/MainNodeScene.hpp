#pragma once

#include "toolgui/Scene.hpp"
#include "NodeEditorApp/SimulationManager.hpp"

namespace node
{
class MainNodeScene;
class Application;
class NodeGraphicsScene;
class GraphicsScene;
class GraphicsObjectsManager;
class BlockClassesManager;
class BlockObject;
class BlockStylerFactory;
class ToolsManager;

namespace loader
{
    class SceneLoader;
}

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

struct DBConnector
{
    std::string db_path;
    std::unique_ptr<node::loader::SceneLoader> connector;
};

class MainNodeScene: public node::Scene
{
public:
    MainNodeScene(SDL_FRect rect, node::Application* parent);
    void OnInit() override;
    NodeGraphicsScene* GetCenterWidget() { return m_centerWidget; }
    ~MainNodeScene() override;
    ToolsManager* GetToolsManager() const { return m_toolsManager.get(); }
    void CheckSimulatorEnded();

    void DeleteEventReceiver(const NodeSceneEventReceiver* handler);
    void AddEventReceiver(std::unique_ptr<NodeSceneEventReceiver> handler) { m_event_receivers.push_back(std::move(handler)); }
    void NewScene();
    void LoadScene(std::string name);
    void SaveScene();
    void SaveScene(std::string name);
    void MaybeSaveScene(std::string name);
protected:
    virtual MI::ClickEvent OnRMBUp(MouseButtonEvent& e) override;
    NodeGraphicsScene* m_centerWidget = nullptr;
private:
    void RunSimulator();
    void OnSimulationEnd(const SimulationEvent& event);
    
    void OnSettingsClicked();

    void InitializeTools();
    void InitializeSidePanel(node::GraphicsScene* gScene);
    
    void OpenPropertiesDialog();
    void OpenPropertiesDialog(BlockObject& object);
    void OpenBlockDialog(BlockObject& block);
    void NewScenePressed();
    void LoadSceneButtonPressed();
    void SaveSceneButtonPressed();

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

    std::optional<DBConnector> m_db_connector;
};

}