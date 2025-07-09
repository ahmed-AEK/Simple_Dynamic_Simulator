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

namespace model
{
    struct BlockPortsUpdate;
}

class GraphicsObjectsManager;
class BlockObject;
class SceneModelManager;
struct BlockResult;
class Dialog;

enum class DialogType
{
    BlockDialog,
    PropertiesDialog,
};

struct DialogSlot
{
    HandlePtrS<Dialog, Widget> dialog;
    DialogType type;
};

struct DBConnector
{
    std::string db_path;
    std::unique_ptr<node::loader::SceneLoader> connector;
};

class SceneManager : public SubScenesManager, public MultiObserver<model::BlockPortsUpdate>
{
public:

    using DialogStore = std::unordered_map<BlockObject*, DialogSlot>;
	SceneManager(std::shared_ptr<BlockStylerFactory> block_styler_factory);
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    ~SceneManager() override;

    SubSceneId GetMainSubSceneId() const { return SubSceneId{ 1 }; }
    std::shared_ptr<GraphicsObjectsManager> GetManager(SubSceneId id);
    node::SceneManager::DialogStore& GetDialogs(SubSceneId id);

    void AddModel(std::shared_ptr<SceneModelManager> model);
    std::shared_ptr<SceneModelManager> GetModel(SubSceneId subscene_id) const;

    void SetMainSceneManager(std::shared_ptr<GraphicsObjectsManager> manager);

    void SetSubSceneManager(SubSceneId scene_id, std::shared_ptr<GraphicsObjectsManager> manager);

    void SetDBConnector(std::optional<DBConnector> conn);
    node::DBConnector* GetDBConnector();
    
    std::unordered_map<SubSceneId, std::shared_ptr<GraphicsObjectsManager>>& GetManagers() { return m_managers; }
    
    const std::vector<BlockResult>& GetLastSimulationResults() const { return m_last_sim_results; }
    void SetLastSimulationResults(std::vector<BlockResult> results);

    SubSceneId AddNewSubSceneToScene() override;
    
    void OnNotify(model::BlockPortsUpdate& report) override;

    const auto& GetModels() const
    {
        return m_models;
    }

private:
    SubSceneId m_next_subscene_id{ 2 };
    std::shared_ptr<BlockStylerFactory> m_blockStyleFactory;
    std::unordered_map<SubSceneId, std::shared_ptr<SceneModelManager>> m_models;
    std::unordered_map<SubSceneId, std::shared_ptr<GraphicsObjectsManager>> m_managers;
    std::unordered_map<SubSceneId, std::unordered_map<BlockObject*, DialogSlot>> m_objects_dialogs;
    std::vector<BlockResult> m_last_sim_results;
    std::optional<DBConnector> m_db_connector;
    logging::Logger m_logger = logger(logging::LogCategory::Core);
};

}
