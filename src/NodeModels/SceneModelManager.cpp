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
	auto net_id = model::NetId{ max_id + 1 };
	net->SetId(net_id);
	m_scene->AddNet(net);
	for (auto&& conn : net->GetSocketConnections())
	{
		auto block = m_scene->GetBlockById(conn.socketId.block_id);
		auto sock = block->GetSocketById(conn.socketId.socket_id);
		if (sock)
		{
			sock->get().SetConnectedNetNode(model::NetNodeUniqueId{ conn.NodeId, net_id });
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
		for (const auto& socket : block->GetSockets())
		{
			auto connected_node = socket.GetConnectedNetNode();
			if (connected_node)
			{
				auto net = m_scene->GetNetById(connected_node->net_id);
				assert(net);
				if (net)
				{
					net->get()->RemoveSocketConnectionForSocket(model::SocketUniqueId{ socket.GetId(), id });
				}
			}
		}
		m_scene->RemoveBlockById(id);
		Notify(SceneModification{ SceneModificationType::BlockRemoved, SceneModification::data_t{std::move(block)} });
	}
}

static void MoveNodeAndConnectedNodes(const node::model::NetNodeUniqueId& node_Id, const node::model::Point& point, 
	node::model::NodeSceneModel& model)
{
	using namespace node::model;
	auto net_wrap = model.GetNetById(node_Id.net_id);
	assert(net_wrap);
	if (!net_wrap)
	{
		return;
	}

	NetModel& net = *net_wrap->get();
	auto main_node_wrap = net.GetNetNodeById(node_Id.node_id);
	assert(main_node_wrap);
	if (!main_node_wrap)
	{
		return;
	}

	NetNodeModel& main_node = (*main_node_wrap);
	main_node.SetPosition(point);
	
	auto correct_segment_at = [&](NetNodeModel::ConnectedSegmentSide side) {
		auto east_segment = main_node.GetSegmentAt(side);
		std::optional<NetNodeId> other_side{ std::nullopt };
		if (east_segment)
		{
			auto segment_model = net.GetNetSegmentById(*east_segment);
			if (segment_model)
			{
				other_side = segment_model->get().m_firstNodeId;
				if (other_side == node_Id.node_id)
				{
					other_side = segment_model->get().m_secondNodeId;
				}
			}
		}
		if (other_side)
		{
			auto other_node = net.GetNetNodeById(*other_side);
			if (other_node)
			{
				other_node->get().SetPosition({ other_node->get().GetPosition().x, point.y });
			}
		}
		};

	correct_segment_at(NetNodeModel::ConnectedSegmentSide::east);
	correct_segment_at(NetNodeModel::ConnectedSegmentSide::west);
	assert(!main_node.GetSegmentAt(NetNodeModel::ConnectedSegmentSide::north)); // TODO
	assert(!main_node.GetSegmentAt(NetNodeModel::ConnectedSegmentSide::south)); // TODO
}

void node::SceneModelManager::MoveBlockById(const model::BlockId& id, const model::Point& new_origin)
{
	auto block = m_scene->GetBlockById(id);
	assert(block);
	if (block)
	{
		block->SetPosition(new_origin);
		for (auto&& socket : block->GetSockets())
		{
			const auto& connected_node = socket.GetConnectedNetNode();
			if (connected_node)
			{
				MoveNodeAndConnectedNodes(*connected_node, new_origin, *m_scene);
			}
		}
		Notify(SceneModification{ SceneModificationType::BlockMoved, SceneModification::data_t{std::move(block)} });
	}
}
