#pragma once


#include "toolgui/Widget.hpp"
#include "NodeEditorApp/IdTypes.hpp"
#include "NodeModels/IdTypes.hpp"
#include "SceneLoader/SceneLoader.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"
#include <optional>
#include <memory>
#include <unordered_map>

namespace node
{

namespace loader
{
    class SceneLoader;
}

class GraphicsObjectsManager;
class BlockObject;
class SceneModelManager;
struct BlockResult;

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

class SceneManager : public SubScenesManager
{
public:

    using DialogStore = std::unordered_map<BlockObject*, DialogSlot>;
	SceneManager();
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    ~SceneManager();

    SubSceneId GetMainSubSceneId() const { return SubSceneId{ 1 }; }
    std::shared_ptr<GraphicsObjectsManager> GetManager(SubSceneId id);
    node::SceneManager::DialogStore& GetDialogs(SubSceneId id);

    void SetModel(SubSceneId subscene_id, std::shared_ptr<SceneModelManager> model);
    std::shared_ptr<SceneModelManager> GetModel(SubSceneId subscene_id) const;

    void SetMainSceneManager(std::shared_ptr<GraphicsObjectsManager> manager);
    void SetMainSceneModel(std::shared_ptr<SceneModelManager> manager);
    
    void SetDBConnector(std::optional<DBConnector> conn);
    std::optional<node::DBConnector>& GetDBConnector();
    
    std::unordered_map<SubSceneId, std::shared_ptr<GraphicsObjectsManager>>& GetManagers() { return m_managers; }
    
    const std::vector<BlockResult>& GetLastSimulationResults() const { return m_last_sim_results; }
    void SetLastSimulationResults(std::vector<BlockResult> results);

    SubSceneId AddNewSubSceneToScene(node::SubSceneId parent_id) override;

private:
    SubSceneId m_next_subscene_id{ 2 };

    std::unordered_map<SubSceneId, std::shared_ptr<SceneModelManager>> m_models;
    std::unordered_map<SubSceneId, std::shared_ptr<GraphicsObjectsManager>> m_managers;
    std::unordered_map<SubSceneId, std::unordered_map<BlockObject*, DialogSlot>> m_objects_dialogs;
    std::vector<BlockResult> m_last_sim_results;
    std::optional<DBConnector> m_db_connector;
};

}
