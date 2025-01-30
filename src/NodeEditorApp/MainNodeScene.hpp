#pragma once

#include "toolgui/Scene.hpp"
#include "NodeEditorApp/SimulationManager.hpp"
#include "NodeEditorApp/IdTypes.hpp"

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
class TabbedView;
class SceneManager;
class SceneGrid;

class NodeSceneEventReceiver
{
public:
    explicit NodeSceneEventReceiver(MainNodeScene& scene) : m_scene{ &scene } {}
    virtual ~NodeSceneEventReceiver() = default;
    MainNodeScene* GetScene() { return m_scene; }
private:
    MainNodeScene* m_scene;
};

struct TabChangeNotifier;

class MainNodeScene: public node::Scene
{
public:
    MainNodeScene(const WidgetSize& size, node::Application* parent);
    void OnInit() override;
    ~MainNodeScene() override;
    ToolsManager* GetToolsManager() const { return m_toolsManager.get(); }

    void DeleteEventReceiver(const NodeSceneEventReceiver* handler);
    void AddEventReceiver(std::unique_ptr<NodeSceneEventReceiver> handler) { m_event_receivers.push_back(std::move(handler)); }
    void NewScene();
    void LoadScene(std::string name);
    void SaveScene();
    void SaveScene(std::string name);
    void MaybeSaveScene(std::string name);
    void OnCenterTabChanged(int32_t new_tab);
    void CloseTabRequest(int32_t tab_idx);
protected:
    virtual MI::ClickEvent OnRMBUp(MouseButtonEvent& e) override;
    HandlePtrS<TabbedView,Widget> m_tabbedView;
    HandlePtrS<SceneGrid, Widget> m_scene_grid;
private:
    void RunSimulator();
    void OnSimulationEnd(const SimulationEvent& event);
    
    void OnSettingsClicked();
    void OnAboutClicked();

    void InitializeTools();
    void InitializeSidePanel();
    
    void OpenPropertiesDialog();
    void OpenPropertiesDialog(BlockObject& object);
    void OpenBlockDialog(BlockObject& block);
    void NewScenePressed();
    void LoadSceneButtonPressed();
    void SaveSceneButtonPressed();
    SceneManagerId CreateNewScene();
    bool CreateSceneForSubsystem(SceneId scene_id);

    void OnUndo();
    void OnRedo();
    bool CanUndo();
    bool CanRedo();

    SceneManagerId GetNewManagerId()
    {
        SceneManagerId new_id = SceneManagerId{ m_next_manager_id };
        m_next_manager_id++;
        return new_id;
    }

    std::optional<node::SceneId> GetSceneIdForTab(int32_t index);

    struct SceneComponents
    {
        std::unique_ptr<SceneManager> manager;
    };

    std::unordered_map<SceneManagerId, SceneComponents> m_sceneComponents;
    std::optional<SceneId> m_current_scene_id;
    int32_t m_next_manager_id = 1;

    std::shared_ptr<ToolsManager> m_toolsManager;
    std::shared_ptr<BlockClassesManager> m_classesManager;
    std::shared_ptr<BlockStylerFactory> m_blockStylerFactory;
    std::vector<std::unique_ptr<NodeSceneEventReceiver>> m_event_receivers;
    std::unique_ptr<TabChangeNotifier> m_tab_change_notifier;

    SimulationManager m_sim_mgr;
    HandlePtrS<Dialog, Widget> m_settings_dialog;
    HandlePtrS<Dialog, Widget> m_about_dialog;


};

}