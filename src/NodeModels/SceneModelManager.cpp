#include "SceneModelManager.hpp"

node::SceneModelManager::SceneModelManager(std::shared_ptr<model::NodeSceneModel> scene)
	:m_scene(std::move(scene))
{
}

node::SceneModelManager::~SceneModelManager()
{
}

std::span<node::model::BlockModelPtr> node::SceneModelManager::GetBlocks()
{
	return m_scene->GetBlocks();
}

void node::SceneModelManager::AddBlock(model::BlockModelPtr block)
{
	model::id_int max_id = 0;
	for (auto&& it_block : m_scene->GetBlocks())
	{
		max_id = std::max(max_id, it_block->GetId());
	}
	block->SetId(max_id + 1);
	m_scene->AddBlock(block);
	Notify(SceneModification{ SceneModificationType::BlockAdded, SceneModification::data_t{std::move(block)} });
}

void node::SceneModelManager::RemoveBlockById(model::id_int id)
{
	auto block = m_scene->GetBlockById(id);
	assert(block);
	if (block)
	{
		m_scene->RemoveBlockById(id);
		Notify(SceneModification{ SceneModificationType::BlockRemoved, SceneModification::data_t{std::move(block)} });
	}
}
