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

void node::SceneModelManager::AddNewBlock(model::BlockModelPtr block)
{
	model::id_int max_id = 0;
	for (auto&& it_block : m_scene->GetBlocks())
	{
		max_id = std::max(max_id, it_block->GetId().value);
	}
	block->SetId(model::BlockId{ max_id + 1 });
	m_scene->AddBlock(block);
	Notify(SceneModification{ SceneModificationType::BlockAdded, SceneModification::data_t{std::move(block)} });
}

void node::SceneModelManager::AddNewNet(model::NetModelPtr net)
{
	assert(m_scene);
	model::id_int max_id = 0;
	for (auto&& it_net : m_scene->GetNets())
	{
		max_id = std::max(max_id, it_net->GetId().value);
	}
	net->SetId(model::NetId{ max_id + 1 });
	m_scene->AddNet(net);
	for (auto&& conn : net->GetSocketConnections())
	{
		auto block = m_scene->GetBlockById(conn.socketId.block_id);
		auto sock = block->GetSocketById(conn.socketId.socket_id);
		if (sock)
		{
			(*sock).get().SetConnectedNetNode(conn.NodeId);
		}
	}
	Notify(SceneModification{ SceneModificationType::NetAdded, SceneModification::data_t{net} });
}

void node::SceneModelManager::RemoveBlockById(const model::BlockId& id)
{
	auto block = m_scene->GetBlockById(id);
	assert(block);
	if (block)
	{
		m_scene->RemoveBlockById(id);
		Notify(SceneModification{ SceneModificationType::BlockRemoved, SceneModification::data_t{std::move(block)} });
	}
}

void node::SceneModelManager::MoveBlockById(const model::BlockId& id, const model::Point& new_origin)
{
	auto block = m_scene->GetBlockById(id);
	assert(block);
	if (block)
	{
		block->SetPosition(new_origin);
		Notify(SceneModification{ SceneModificationType::BlockMoved, SceneModification::data_t{std::move(block)} });
	}
}
