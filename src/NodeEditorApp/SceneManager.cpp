#include "SceneManager.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"
#include "NodeEditorApp/SimulatorRunner.hpp"

node::SceneManager::SceneManager()
{
}

node::SceneManager::~SceneManager()
{
}

std::shared_ptr<node::GraphicsObjectsManager> node::SceneManager::GetManager(SubSceneId id)
{
	auto it = m_managers.find(id);
	if (it != m_managers.end())
	{
		return it->second;
	}
	return nullptr;
}

node::SceneManager::DialogStore& node::SceneManager::GetDialogs(SubSceneId id)
{
	return m_objects_dialogs[id];
}

void node::SceneManager::SetModel(SubSceneId subscene_id, std::shared_ptr<SceneModelManager> model)
{
	assert(m_models.find(subscene_id) == m_models.end());
	m_models[subscene_id] = model;
}

std::shared_ptr<node::SceneModelManager> node::SceneManager::GetModel(SubSceneId subscene_id) const
{
	auto it = m_models.find(subscene_id);
	if (it != m_models.end())
	{
		return it->second;
	}
	return nullptr;
}

void node::SceneManager::SetMainSceneManager(std::shared_ptr<GraphicsObjectsManager> manager)
{
	m_managers[GetMainSubSceneId()] = manager;
}

void node::SceneManager::SetMainSceneModel(std::shared_ptr<SceneModelManager> manager)
{
	auto it = m_managers.find(GetMainSubSceneId());
	assert(it != m_managers.end());
	if (it != m_managers.end())
	{
		it->second->SetSceneModel(manager);
	}
}

void node::SceneManager::SetDBConnector(std::optional<DBConnector> conn)
{
	m_db_connector = std::move(conn);
}

std::optional<node::DBConnector>& node::SceneManager::GetDBConnector()
{
	return m_db_connector;
}

void node::SceneManager::SetLastSimulationResults(std::vector<BlockResult> results)
{
	m_last_sim_results = std::move(results);
}