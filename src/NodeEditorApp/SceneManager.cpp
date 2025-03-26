#include "SceneManager.hpp"
#include "GraphicsScene/GraphicsObjectsManager.hpp"
#include "NodeEditorApp/SimulatorRunner.hpp"
#include "NodeModels/BlockPortsUpdate.hpp"
#include "NodeSDLStylers/BlockStylerFactory.hpp"

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

void node::SceneManager::AddModel(std::shared_ptr<SceneModelManager> model)
{
	auto subscene_id = model->GetSubSceneId();
	assert(m_models.find(subscene_id) == m_models.end());
	m_models[subscene_id] = model;
	static_cast<Publisher<model::BlockPortsUpdate>*>(model.get())->Attach(*static_cast<Observer<model::BlockPortsUpdate>*>(this));
	if (m_next_subscene_id.value <= subscene_id.value)
	{
		m_next_subscene_id = SubSceneId{ subscene_id.value + 1 };
	}
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
	SetSubSceneManager(GetMainSubSceneId(), manager);
}

void node::SceneManager::SetSubSceneManager(SubSceneId scene_id, std::shared_ptr<GraphicsObjectsManager> manager)
{
	if (manager)
	{
		auto model = GetModel(scene_id);
		assert(model);
		manager->SetSceneModel(model);
		manager->SetSubSceneManager(this);
		m_managers[scene_id] = std::move(manager);
	}
	else
	{
		auto it = m_managers.find(scene_id);
		if (it != m_managers.end())
		{
			m_managers.erase(it);
		}
	}
}

void node::SceneManager::SetDBConnector(std::optional<DBConnector> conn)
{
	m_db_connector = std::move(conn);
}

node::DBConnector* node::SceneManager::GetDBConnector()
{
	return m_db_connector ? &(*m_db_connector) : nullptr;
}

void node::SceneManager::SetLastSimulationResults(std::vector<BlockResult> results)
{
	m_last_sim_results = std::move(results);
}

node::SubSceneId node::SceneManager::AddNewSubSceneToScene()
{
	auto id = m_next_subscene_id;
	auto manager = std::make_shared<SceneModelManager>(std::make_shared<model::NodeSceneModel>());
	manager->SetSubSceneId(id);
	AddModel(manager);
	m_next_subscene_id.value++;
	return id;
}

void node::SceneManager::OnNotify(model::BlockPortsUpdate& report)
{
	UNUSED_PARAM(report);
	SDL_Log("updated port %d on scene %d", report.block_id.value, report.scene_id.value);
	std::vector<model::BlockSocketModel> sockets;
	auto models_it = m_models.find(report.scene_id);
	assert(models_it != m_models.end());
	if (models_it == m_models.end())
	{
		return;
	}

	assert(m_blockStyleFactory);

	for (auto&& [block_id, port_data] : models_it->second->GetModel().GetPortBlocksManager().GetData())
	{
		sockets.push_back(model::BlockSocketModel{ port_data.port_type, port_data.id });
	}

	for (auto&& [subscene_id, subscene_model] : m_models)
	{
		for (auto& [block_id, subsystem_data] : subscene_model->GetModel().GetSubsystemBlocksManager().GetData())
		{
			if (subsystem_data.scene_id == report.scene_id)
			{
				// we found a subsystem block that needs to be updated
				auto* block_ptr = subscene_model->GetModel().GetBlockById(block_id);
				assert(block_ptr);
				if (!block_ptr)
				{
					continue;
				}
				const auto& styler_type = block_ptr->GetStyler();
				auto styler = m_blockStyleFactory->GetStyler(styler_type, { *block_ptr, subsystem_data });
				if (!styler)
				{
					continue;
				}
				styler->PositionSockets(sockets, block_ptr->GetBounds(), block_ptr->GetOrienation());
				subscene_model->ModifyBlockSockets(block_id, sockets);
			}
		}
	}

}
