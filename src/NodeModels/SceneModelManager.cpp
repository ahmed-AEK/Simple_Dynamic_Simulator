#include "SceneModelManager.hpp"
#include "BlockPortsUpdate.hpp"
#include "BlockData.hpp"

#include <ranges>

static node::model::NetNodeId GetMaxNodeId(std::span<node::model::NetNodeModel> vec)
{
	using namespace node;
	using namespace model;

	model::id_int max_id = 0;
	for (auto&& it_net : vec)
	{
		max_id = std::max(max_id, it_net.GetId().value);
	}
	return model::NetNodeId{ max_id };
}

static node::model::NetId GetMaxNetId(std::span<node::model::NetModel> vec)
{
	using namespace node;
	using namespace model;

	model::id_int max_id = 0;
	for (auto&& it_net : vec)
	{
		max_id = std::max(max_id, it_net.GetId().value);
	}
	return model::NetId{ max_id };
}

static node::model::NetSegmentId GetMaxSegmentId(std::span<node::model::NetSegmentModel> vec)
{
	using namespace node;
	using namespace model;

	model::id_int max_id = 0;
	for (auto&& it_net : vec)
	{
		max_id = std::max(max_id, it_net.GetId().value);
	}
	return model::NetSegmentId{ max_id };
}

namespace node
{

struct AddSubsystemBlockAction : public ModelAction
{
	explicit AddSubsystemBlockAction(model::BlockModel&& block, model::SubsystemBlockData&& data)
		:block{ std::move(block) }, data{ std::move(data) } {}
	const model::BlockModel block;
	std::optional<model::BlockId> stored_id;
	const model::SubsystemBlockData data;

	bool DoUndo(SceneModelManager& manager) override
	{
		assert(stored_id);
		if (!stored_id)
		{
			return false;
		}

		auto* block_ptr= manager.GetModel().GetBlockById(*stored_id);
		assert(block_ptr);
		if (!block_ptr)
		{
			return false;
		}
		auto& scene_model = manager.GetModel();
		scene_model.RemoveBlockById(*stored_id);
		[[maybe_unused]] auto success = scene_model.GetSubsystemBlocksManager().EraseDataForId(*stored_id);
		assert(success);

		manager.Notify(SceneModification{ SceneModificationType::BlockRemoved, SceneModification::data_t{*stored_id} });

		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		model::id_int max_id = 0;
		for (auto&& it_block : manager.GetModel().GetBlocks())
		{
			max_id = std::max(max_id, it_block.GetId().value);
		}
		model::BlockId block_id{ max_id + 1 };
		stored_id = block_id;

		model::BlockModel temp_block = block;
		temp_block.SetId(block_id);

		auto& scene_model = manager.GetModel();
		scene_model.AddBlock(std::move(temp_block));
		scene_model.GetSubsystemBlocksManager().SetDataForId(block_id, model::SubsystemBlockData{ data });
		auto* block_ptr = manager.GetModel().GetBlockById(block_id);
		assert(block_ptr);
		manager.Notify(SceneModification{ SceneModificationType::BlockAdded, SceneModification::data_t{*block_ptr} });

		return true;
	}
};


struct AddPortBlockAction : public ModelAction
{
	explicit AddPortBlockAction(model::BlockModel&& block, model::PortBlockData&& data)
		:block{ std::move(block) }, data{ std::move(data) } {}
	const model::BlockModel block;
	std::optional<model::BlockId> stored_id;
	const model::PortBlockData data;

	bool DoUndo(SceneModelManager& manager) override
	{
		assert(stored_id);
		if (!stored_id)
		{
			return false;
		}

		auto* block_ptr = manager.GetModel().GetBlockById(*stored_id);
		assert(block_ptr);
		if (!block_ptr)
		{
			return false;
		}
		auto& scene_model = manager.GetModel();
		scene_model.RemoveBlockById(*stored_id);
		[[maybe_unused]] auto success = scene_model.GetPortBlocksManager().RemovePortForId(*stored_id);
		assert(success);

		manager.Notify(SceneModification{ SceneModificationType::BlockRemoved, SceneModification::data_t{*stored_id} });
		manager.Notify(model::BlockPortsUpdate{ manager.GetSubSceneId(), block_ptr->GetId() });
		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		model::id_int max_id = 0;
		for (auto&& it_block : manager.GetModel().GetBlocks())
		{
			max_id = std::max(max_id, it_block.GetId().value);
		}
		model::BlockId block_id{ max_id + 1 };
		stored_id = block_id;

		model::BlockModel temp_block = block;
		temp_block.SetId(block_id);

		auto& scene_model = manager.GetModel();
		scene_model.AddBlock(std::move(temp_block));
		scene_model.GetPortBlocksManager().AddPortForId(block_id, data.port_type);
		auto* block_ptr = manager.GetModel().GetBlockById(block_id);
		assert(block_ptr);
		manager.Notify(SceneModification{ SceneModificationType::BlockAdded, SceneModification::data_t{*block_ptr} });
		manager.Notify(model::BlockPortsUpdate{ manager.GetSubSceneId(), block_ptr->GetId() });
		return true;
	}
};


struct AddFuncitonalBlockAction : public ModelAction
{
	explicit AddFuncitonalBlockAction(model::BlockModel&& block, model::FunctionalBlockData&& data)
		:block{ std::move(block) }, data{ std::move(data) } {}
	const model::BlockModel block;
	std::optional<model::BlockId> stored_id;
	const model::FunctionalBlockData data;

	bool DoUndo(SceneModelManager& manager) override
	{
		assert(stored_id);
		if (!stored_id)
		{
			return false;
		}

		auto* block_ptr = manager.GetModel().GetBlockById(*stored_id);
		assert(block_ptr);
		if (!block_ptr)
		{
			return false;
		}
		auto& scene_model = manager.GetModel();
		scene_model.RemoveBlockById(*stored_id);
		[[maybe_unused]] auto success = scene_model.GetFunctionalBlocksManager().EraseDataForId(*stored_id);
		assert(success);

		manager.Notify(SceneModification{ SceneModificationType::BlockRemoved, SceneModification::data_t{*stored_id} });

		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		model::id_int max_id = 0;
		for (auto&& it_block : manager.GetModel().GetBlocks())
		{
			max_id = std::max(max_id, it_block.GetId().value);
		}
		model::BlockId block_id{ max_id + 1 };
		stored_id = block_id;

		model::BlockModel temp_block = block;
		temp_block.SetId(block_id);

		auto& scene_model = manager.GetModel();
		scene_model.AddBlock(std::move(temp_block));
		scene_model.GetFunctionalBlocksManager().SetDataForId(block_id, model::FunctionalBlockData{ data });
		auto* block_ptr = manager.GetModel().GetBlockById(block_id);
		assert(block_ptr);
		manager.Notify(SceneModification{ SceneModificationType::BlockAdded, SceneModification::data_t{*block_ptr} });
		
		return true;
	}
};


struct RemoveBlockAction : public ModelAction
{
	explicit RemoveBlockAction(model::BlockId id)
		:id{ id } {}
	const model::BlockId id;
	std::optional<model::BlockModel> stored_block;
	std::vector<model::SocketNodeConnection> stored_connections;
	model::BlockData block_data;

	bool DoUndo(SceneModelManager& manager) override
	{
		assert(stored_block);
		manager.GetModel().AddBlock(model::BlockModel{ *stored_block });

		if (stored_block->GetType() == model::BlockType::Functional)
		{
			auto* functional_data = block_data.GetFunctionalData();
			assert(functional_data);
			if (functional_data)
			{
				manager.GetModel().GetFunctionalBlocksManager().SetDataForId(stored_block->GetId(), model::FunctionalBlockData{ *functional_data });
			}
		}
		else if (stored_block->GetType() == model::BlockType::SubSystem)
		{
			auto* subsystem_data = block_data.GetSubsystemData();
			assert(subsystem_data);
			if (subsystem_data)
			{
				manager.GetModel().GetSubsystemBlocksManager().SetDataForId(stored_block->GetId(), model::SubsystemBlockData{ *subsystem_data });
			}
		}
		else if (stored_block->GetType() == model::BlockType::Port)
		{
			auto* port_data = block_data.GetPortData();
			assert(port_data);
			if (port_data)
			{
				manager.GetModel().GetPortBlocksManager().AddPortForId(stored_block->GetId(), port_data->port_type);
			}
		}
		else
		{
			assert(false);
		}
		auto* block_ptr = manager.GetModel().GetBlockById(stored_block->GetId());
		assert(block_ptr);
		if (!block_ptr)
		{
			return false;
		}

		for (auto&& connection : stored_connections)
		{
			manager.GetModel().AddSocketNodeConnection(connection);
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockAddedWithConnections, SceneModification::data_t{BlockAddWithConnectionsReport{*block_ptr,stored_connections} } });
		if (stored_block->GetType() == model::BlockType::Port)
		{
			manager.Notify(model::BlockPortsUpdate{ manager.GetSubSceneId(), stored_block->GetId() });
		}
		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		auto* block = manager.GetModel().GetBlockById(id);
		assert(block);
		if (!block)
		{
			return false;
		}

		stored_connections.clear();
		for (const auto& socket : block->GetSockets())
		{
			auto connected_node = socket.GetConnectedNetNode();
			if (connected_node)
			{
				stored_connections.push_back(*manager.GetModel().GetSocketConnectionForNode(*connected_node));
				manager.GetModel().RemoveSocketConnectionForSocket({ socket.GetId(), block->GetId() });
			}
		}
		stored_block = *block;

		if (block->GetType() == model::BlockType::Functional)
		{
			auto* data_ptr = manager.GetModel().GetFunctionalBlocksManager().GetDataForId(block->GetId());
			assert(data_ptr);
			if (data_ptr)
			{
				block_data = model::BlockData{ std::move(*data_ptr) };
				manager.GetModel().GetFunctionalBlocksManager().EraseDataForId(block->GetId());
			}
		}
		else if (block->GetType() == model::BlockType::SubSystem)
		{
			auto* data_ptr = manager.GetModel().GetSubsystemBlocksManager().GetDataForId(block->GetId());
			assert(data_ptr);
			if (data_ptr)
			{
				block_data = model::BlockData{ std::move(*data_ptr) };
				manager.GetModel().GetSubsystemBlocksManager().EraseDataForId(block->GetId());
			}
		}
		else if (block->GetType() == model::BlockType::Port)
		{
			auto* data_ptr = manager.GetModel().GetPortBlocksManager().GetDataForId(block->GetId());
			assert(data_ptr);
			if (data_ptr)
			{
				block_data = model::BlockData{ std::move(*data_ptr) };
				manager.GetModel().GetPortBlocksManager().RemovePortForId(block->GetId());
			}
		}
		else
		{
			assert(false);
		}
		
		manager.GetModel().RemoveBlockById(id);
		manager.Notify(SceneModification{ SceneModificationType::BlockRemoved, SceneModification::data_t{id} });
		if (stored_block->GetType() == model::BlockType::Port)
		{
			manager.Notify(model::BlockPortsUpdate{ manager.GetSubSceneId(), stored_block->GetId() });
		}
		return true;
	}
};

struct ModifyBlockPropertiesAction : public ModelAction
{
	ModifyBlockPropertiesAction(model::BlockId block_id, std::vector<model::BlockProperty> new_properties)
		:block_id{ block_id }, new_properties{ std::move(new_properties) } {}
	model::BlockId block_id;
	std::vector<model::BlockProperty> new_properties;
	std::vector<model::BlockProperty> old_properties;

	bool DoUndo(SceneModelManager& manager) override
	{
		auto* block = manager.GetModel().GetBlockById(block_id);
		assert(block);
		if (!block)
		{
			return false;
		}
		if (block->GetType() != model::BlockType::Functional)
		{
			return false;
		}

		auto& functional_blocks_manager = manager.GetModel().GetFunctionalBlocksManager();
		auto* block_data = functional_blocks_manager.GetDataForId(block_id);
		if (!block_data)
		{
			return false;
		}
		block_data->properties = old_properties;

		manager.Notify(SceneModification{ SceneModificationType::BlockPropertiesModified , SceneModification::data_t{*block} });
		
		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		auto* block = manager.GetModel().GetBlockById(block_id);
		assert(block);
		if (!block)
		{
			return false;
		}

		if (block->GetType() != model::BlockType::Functional)
		{
			return false;
		}

		auto& functional_blocks_manager = manager.GetModel().GetFunctionalBlocksManager();
		auto* block_data = functional_blocks_manager.GetDataForId(block_id);
		if (!block_data)
		{
			return false;
		}

		old_properties = block_data->properties;
		block_data->properties = new_properties;
		manager.Notify(SceneModification{ SceneModificationType::BlockPropertiesModified , SceneModification::data_t{*block} });
		
		return true;
	}
};

struct ModifyBlockPropertiesAndSocketsAction : public ModelAction
{
	ModifyBlockPropertiesAndSocketsAction(model::BlockId block_id, std::vector<model::BlockProperty> new_properties,
		std::vector<model::BlockSocketModel> new_sockets)
		:block_id{ block_id }, new_properties{ std::move(new_properties) }, new_sockets{ std::move(new_sockets) } {}
	const model::BlockId block_id;
	const std::vector<model::BlockProperty> new_properties;
	std::vector<model::BlockProperty> old_properties;
	const std::vector<model::BlockSocketModel> new_sockets;
	std::vector<model::BlockSocketModel> old_sockets;
	std::vector<model::SocketNodeConnection> stored_connections;

	bool DoUndo(SceneModelManager& manager) override
	{
		auto* block = manager.GetModel().GetBlockById(block_id);
		assert(block);
		if (!block)
		{
			return false;
		}

		if (block->GetType() != model::BlockType::Functional)
		{
			return false;
		}

		auto& functional_blocks_manager = manager.GetModel().GetFunctionalBlocksManager();
		auto* block_data = functional_blocks_manager.GetDataForId(block_id);
		if (!block_data)
		{
			return false;
		}

		block_data->properties = old_properties;

		for (auto&& connection : stored_connections)
		{
			manager.GetModel().AddSocketNodeConnection(connection);
		}

		block->ClearSockets();
		block->ReserveSockets(old_sockets.size());
		for (auto&& socket : old_sockets)
		{
			block->AddSocket(std::move(socket));
		}

		manager.Notify(SceneModification{ SceneModificationType::BlockPropertiesAndSocketsModified , SceneModification::data_t{*block} });

		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		auto* block = manager.GetModel().GetBlockById(block_id);
		assert(block);
		if (!block)
		{
			return false;
		}

		if (block->GetType() != model::BlockType::Functional)
		{
			return false;
		}

		auto& functional_blocks_manager = manager.GetModel().GetFunctionalBlocksManager();
		auto* block_data = functional_blocks_manager.GetDataForId(block_id);
		if (!block_data)
		{
			return false;
		}

		old_properties = block_data->properties;
		block_data->properties = new_properties;

		for (const auto& socket : block->GetSockets())
		{
			if (auto node_id = socket.GetConnectedNetNode())
			{
				stored_connections.push_back(*manager.GetModel().GetSocketConnectionForNode(*node_id));
				manager.GetModel().RemoveSocketConnectionForSocket({ socket.GetId(), block->GetId() });
			}
		}
		auto&& old_span = block->GetSockets();

		old_sockets.clear();
		old_sockets.reserve(old_span.size());
		old_sockets.insert(old_sockets.end(), old_span.begin(), old_span.end());

		block->ClearSockets();
		block->ReserveSockets(new_sockets.size());
		for (auto&& socket : new_sockets)
		{
			block->AddSocket(socket);
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockPropertiesAndSocketsModified , SceneModification::data_t{*block} });

		return true;
	}
};

struct ModifyBlockSocketsAction : public ModelAction
{
	ModifyBlockSocketsAction(model::BlockId block_id,
		std::vector<model::BlockSocketModel> new_sockets)
		:block_id{ block_id }, new_sockets{ std::move(new_sockets) } {}
	const model::BlockId block_id;
	const std::vector<model::BlockSocketModel> new_sockets;
	std::vector<model::BlockSocketModel> old_sockets;
	std::vector<model::SocketNodeConnection> stored_connections;

	bool DoUndo(SceneModelManager& manager) override
	{
		auto* block = manager.GetModel().GetBlockById(block_id);
		assert(block);
		if (!block)
		{
			return false;
		}

		for (auto&& connection : stored_connections)
		{
			manager.GetModel().AddSocketNodeConnection(connection);
		}

		block->ClearSockets();
		block->ReserveSockets(old_sockets.size());
		for (auto&& socket : old_sockets)
		{
			block->AddSocket(std::move(socket));
		}

		manager.Notify(SceneModification{ SceneModificationType::BlockPropertiesAndSocketsModified , SceneModification::data_t{*block} });

		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		auto* block = manager.GetModel().GetBlockById(block_id);
		assert(block);
		if (!block)
		{
			return false;
		}

		for (const auto& socket : block->GetSockets())
		{
			if (auto node_id = socket.GetConnectedNetNode())
			{
				stored_connections.push_back(*manager.GetModel().GetSocketConnectionForNode(*node_id));
				manager.GetModel().RemoveSocketConnectionForSocket({ socket.GetId(), block->GetId() });
			}
		}
		auto&& old_span = block->GetSockets();

		old_sockets.clear();
		old_sockets.reserve(old_span.size());
		old_sockets.insert(old_sockets.end(), old_span.begin(), old_span.end());

		block->ClearSockets();
		block->ReserveSockets(new_sockets.size());
		for (auto&& socket : new_sockets)
		{
			block->AddSocket(socket);
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockPropertiesAndSocketsModified , SceneModification::data_t{*block} });

		return true;
	}
};

struct UpdateNetAction : public ModelAction
{
	explicit UpdateNetAction(NetModificationRequest&& update_request)
		:update_request{ std::move(update_request) } {}

	const NetModificationRequest update_request;

	struct StoredSegment
	{
		model::NetSegmentModel segment;
		model::ConnectedSegmentSide first_side;
		model::ConnectedSegmentSide second_side;
	};

	struct StoredNodeUpdates
	{
		model::NetNodeId id;
		model::Point old_position;
	};

	struct StoredSegmentUpdate
	{
		model::NetSegmentId id;
		model::NetNodeId old_node1;
		model::NetNodeId old_node2;
		model::ConnectedSegmentSide old_first_side;
		model::ConnectedSegmentSide old_second_side;
		model::NetSegmentOrientation old_orientation;
	};

	struct StoredAddedSegment
	{
		model::NetSegmentId id;
		model::ConnectedSegmentSide first_side;
		model::ConnectedSegmentSide second_side;
	};
	struct StoredNetCategoryChange
	{
		model::NetId net_id;
		model::NetCategory old_category;
	};

	struct StoredNetMerge
	{
		model::NetId net1_id;
		model::NetCategory net1_old_category;
		model::NetModel second_net;
	};

	struct StoredNodeNetChange
	{
		model::NetNodeId node;
		model::NetId net;
	};

	struct StoredEdits
	{
		std::vector<model::SocketNodeConnection> stored_deleted_connections;
		std::vector<StoredSegment> stored_deleted_segments;
		std::vector<model::NetNodeModel> stored_deleted_nodes;
		std::vector<model::NetNodeId> stored_added_nodes;
		std::vector<StoredNodeUpdates> stored_node_updates;
		std::vector<StoredSegmentUpdate> stored_segment_updates;
		std::vector<StoredAddedSegment> stored_new_segments;
		std::vector<model::SocketNodeConnection> stored_new_connections;
		std::vector<model::NetId> stored_new_nets;
		std::vector<StoredNetCategoryChange> stored_net_category_change;
		std::vector<StoredNetMerge> stored_merged_net;
		std::vector<model::NetModel> stored_deleted_nets;
		std::vector< StoredNodeNetChange> stored_node_net_change;
	};

	StoredEdits edits;

	bool DoUndo(SceneModelManager& manager) override
	{
		auto&& scene = manager.GetModel();
		// remove added connections
		for (auto&& connection : edits.stored_new_connections)
		{
			auto* block = scene.GetBlockById(connection.socketId.block_id);
			assert(block);
			if (!block)
			{
				return false;
			}
			auto* socket = block->GetSocketById(connection.socketId.socket_id);
			assert(socket);
			socket->SetConnectedNetNode(std::nullopt);
			scene.RemoveSocketConnectionForSocket(connection.socketId);
		}

		// remove added segments
		for (auto&& segment_info : edits.stored_new_segments)
		{

			auto&& segment = scene.GetNetSegmentById(segment_info.id);
			if (!segment)
			{
				return false;
			}

			auto* node1 = scene.GetNetNodeById(segment->m_firstNodeId);
			assert(node1);
			auto* node2 = scene.GetNetNodeById(segment->m_secondNodeId);
			assert(node2);
			if (!node1 || !node2)
			{
				return false;
			}
			scene.RemoveNetSegmentById(segment_info.id);
			node1->SetSegmentAt(segment_info.first_side,std::nullopt);
			node2->SetSegmentAt(segment_info.second_side, std::nullopt);
		}

		// undo updated segments
		for (auto&& segment_request : edits.stored_segment_updates)
		{
			auto* updated_segment = scene.GetNetSegmentById(segment_request.id);
			assert(updated_segment);
			if (!updated_segment)
			{
				return false;
			}

			{
				// disconnect from old node
				auto node_id = updated_segment->m_firstNodeId;
				auto* node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && *segment == segment_request.id)
					{
						node->SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			{
				// disconnect from old node
				auto node_id = updated_segment->m_secondNodeId;
				auto* node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && (*segment) == segment_request.id)
					{
						node->SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}

			{
				auto node_id1 = segment_request.old_node1;
				auto* node1 = scene.GetNetNodeById(node_id1);
				assert(node1);
				auto node_id2 = segment_request.old_node2;
				auto* node2 = scene.GetNetNodeById(node_id2);
				assert(node2);

				if (!node1 || !node2)
				{
					return false;
				}

				updated_segment->m_firstNodeId = segment_request.old_node1;
				updated_segment->m_secondNodeId = segment_request.old_node2;
				updated_segment->m_orientation = segment_request.old_orientation;

				node1->SetSegmentAt(segment_request.old_first_side, updated_segment->GetId());
				node2->SetSegmentAt(segment_request.old_second_side, updated_segment->GetId());
			}
		}

		// undo merge nets
		std::vector<NetModificationReport::UpdateNodeNetReport> node_net_changed;
		for (auto&& merged_net : edits.stored_merged_net)
		{
			scene.AddNet(merged_net.second_net);
			auto net1_id = merged_net.net1_id;
			auto net2_id = merged_net.second_net.GetId();
			auto* net1 = scene.GetNet(net1_id);
			assert(net1);

			for (const auto& node_id : merged_net.second_net.GetNodes())
			{
				auto* node = scene.GetNetNodeById(node_id);
				assert(node);
				node->SetNetId(net2_id);
				net1->removeNode(node_id);
			}


			for (const auto& node_id : merged_net.second_net.GetNodes())
			{
				auto it = std::find_if(edits.stored_added_nodes.begin(), edits.stored_added_nodes.end(),
					[&](const model::NetNodeId& target_id) { return target_id == node_id; });
				if (it == edits.stored_added_nodes.end())
				{
					node_net_changed.push_back({ node_id, merged_net.second_net.GetId() });
				}
			}
			if (net1->GetCategory() != merged_net.net1_old_category)
			{
				net1->SetCategory(merged_net.net1_old_category);
				for (auto& node_id : net1->GetNodes())
				{
					auto it = std::find_if(edits.stored_added_nodes.begin(), edits.stored_added_nodes.end(),
						[&](const model::NetNodeId& target_id) { return target_id == node_id; });
					if (it == edits.stored_added_nodes.end())
					{
						node_net_changed.push_back({ node_id, net1->GetId() });
					}
				}
			}
		}

		// undo net category change
		for (auto&& net_change : edits.stored_net_category_change)
		{
			auto* net = scene.GetNet(net_change.net_id);
			assert(net);
			net->SetCategory(net_change.old_category);
			for (auto& node_id : net->GetNodes())
			{
				auto it = std::find_if(edits.stored_added_nodes.begin(), edits.stored_added_nodes.end(), 
					[&](const model::NetNodeId& target_id) { return target_id == node_id; });
				if (it == edits.stored_added_nodes.end())
				{
					node_net_changed.push_back({ node_id, net->GetId() });
				}
			}
		}
		
		// uno node net change
		for (const auto& node_modification : edits.stored_node_net_change)
		{
			auto* node = scene.GetNetNodeById(node_modification.node);
			assert(node);
			auto old_net_id = node->GetNetId();
			auto* old_net = scene.GetNet(old_net_id);
			old_net->removeNode(node_modification.node);
			auto* new_net = scene.GetNet(node_modification.net);
			assert(new_net);
			node->SetNetId(node_modification.net);
			new_net->AddNode(node_modification.node);
			node_net_changed.push_back({ node_modification.node, new_net->GetId() });
		}
		

		// undo updated Nodes
		for (auto&& node_request : edits.stored_node_updates)
		{
			auto* node = scene.GetNetNodeById(node_request.id);
			assert(node);
			if (node)
			{
				node->SetPosition(node_request.old_position);
			}
		}

		// undo added nodes
		for (auto&& node_id : edits.stored_added_nodes)
		{
			auto* node = scene.GetNetNodeById(node_id);
			assert(node);
			auto net_id = node->GetNetId();
			scene.RemoveNetNodeById(node_id);

			// remove from containing net
			auto* net = scene.GetNet(net_id);
			assert(net);
			net->removeNode(node_id);
		}

		// undo added nets
		for (auto&& net_id : edits.stored_new_nets)
		{
			scene.RemoveNetById(net_id);
		}

		{
			// undo deleted nets
			for (const auto& stored_net : edits.stored_deleted_nets)
			{
				scene.AddNet(stored_net);
			}
		}

		// undo deleted nodes
		for (auto&& deleted_node : edits.stored_deleted_nodes)
		{
			scene.AddNetNode(model::NetNodeModel{ deleted_node });
			auto* net = scene.GetNet(deleted_node.GetNetId());
			assert(net);
			net->AddNode(deleted_node.GetId());
		}

		// undo deleted segments
		for (auto&& deleted_segment_report : edits.stored_deleted_segments)
		{
			
			scene.AddNetSegment(model::NetSegmentModel{ deleted_segment_report.segment });
			{
				auto node_id = deleted_segment_report.segment.m_firstNodeId;
				auto* node = scene.GetNetNodeById(node_id);
				assert(node);
				if (!node)
				{
					return false;
				}
				node->SetSegmentAt(deleted_segment_report.first_side, deleted_segment_report.segment.GetId());
			}
			
			{
				auto node_id = deleted_segment_report.segment.m_secondNodeId;
				auto* node = scene.GetNetNodeById(node_id);
				assert(node);
				if (!node)
				{
					return false;
				}
				node->SetSegmentAt(deleted_segment_report.second_side, deleted_segment_report.segment.GetId());
			}
		}

		// undo deleted connections
		for (const auto& deleted_connection_report : edits.stored_deleted_connections)
		{
			auto* block = scene.GetBlockById(deleted_connection_report.socketId.block_id);
			assert(block);
			if (!block)
			{
				return false;
			}
			auto* socket = block->GetSocketById(deleted_connection_report.socketId.socket_id);
			assert(socket);
			if (!socket)
			{
				return false;
			}
			scene.AddSocketNodeConnection(model::SocketNodeConnection{deleted_connection_report.socketId, deleted_connection_report.NodeId});
			socket->SetConnectedNetNode(deleted_connection_report.NodeId);
		}
		
		// setup notification
		NetModificationReport report;
		report.node_net_changed = std::move(node_net_changed);
		{
			auto transform_range = edits.stored_new_connections | std::ranges::views::transform([](const model::SocketNodeConnection& conn) {return conn.socketId; });
			report.removed_connections.insert(report.removed_connections.end(), transform_range.begin(), transform_range.end());
		}

		{
			auto transform_range = edits.stored_new_segments | std::ranges::views::transform([](const StoredAddedSegment& segment)->model::NetSegmentId {return segment.id; });
			report.removed_segments.insert(report.removed_segments.end(), transform_range.begin(), transform_range.end());
		}

		report.removed_nodes = edits.stored_added_nodes;

		report.added_nodes.reserve(edits.stored_deleted_nodes.size());
		for (auto&& added_node : edits.stored_deleted_nodes)
		{
			auto* node = scene.GetNetNodeById(added_node.GetId());
			assert(node);
			report.added_nodes.push_back(*node);
		}

		report.update_nodes.reserve(edits.stored_node_updates.size());
		for (auto&& node_update : edits.stored_node_updates)
		{
			report.update_nodes.push_back({ node_update.id, node_update.old_position });
		}

		report.update_segments.reserve(edits.stored_segment_updates.size());
		for (auto&& update_segment_request : update_request.update_segments)
		{
			auto* segment = scene.GetNetSegmentById(update_segment_request.segment_id);
			assert(segment);
			report.update_segments.push_back(*segment);
		}

		report.added_segments.reserve(edits.stored_deleted_segments.size());
		for (auto&& added_segment : edits.stored_deleted_segments)
		{
			auto* segment = scene.GetNetSegmentById(added_segment.segment.GetId());
			assert(segment);
			report.added_segments.push_back(*segment);
		}

		report.added_connections.reserve(edits.stored_deleted_connections.size());
		for (auto&& added_connection : edits.stored_deleted_connections)
		{
			auto&& node_id = added_connection.NodeId;
			auto* conn = scene.GetSocketConnectionForNode(node_id);
			assert(conn);
			report.added_connections.push_back(*conn);
		}

		manager.Notify(SceneModification{ SceneModification::type_t::NetUpdated, SceneModification::data_t{std::ref(report)} });

		return true;

	}

	bool DoRedo(SceneModelManager& manager) override
	{
		edits.stored_deleted_connections.clear();
		edits.stored_deleted_segments.clear();
		edits.stored_deleted_nodes.clear();
		edits.stored_added_nodes.clear();
		edits.stored_node_updates.clear();
		edits.stored_segment_updates.clear();
		edits.stored_new_segments.clear();
		edits.stored_new_connections.clear();
		edits.stored_new_nets.clear();
		edits.stored_net_category_change.clear();
		edits.stored_merged_net.clear();
		edits.stored_deleted_nets.clear();
		edits.stored_node_net_change.clear();

		auto&& scene = manager.GetModel();
		// handle deleted connections
		for (const auto& deleted_connection : update_request.removed_connections)
		{
			auto&& conn = scene.GetSocketConnectionForSocket(deleted_connection);
			auto* block = scene.GetBlockById(deleted_connection.block_id);
			assert(conn);
			assert(block);
			if (!conn || !block)
			{
				return false;
			}
			auto* socket = block->GetSocketById(deleted_connection.socket_id);
			assert(socket);
			socket->SetConnectedNetNode(std::nullopt);
			model::SocketNodeConnection stored_conn = *conn;
			scene.RemoveSocketConnectionForSocket(deleted_connection);
			edits.stored_deleted_connections.push_back(std::move(stored_conn));
		}

		// handle deleted segments
		for (auto&& deleted_segment_id : update_request.removed_segments)
		{
			auto&& deleted_segment = scene.GetNetSegmentById(deleted_segment_id);
			assert(deleted_segment);
			if (!deleted_segment)
			{
				return false;
			}
			StoredSegment stored_segment{ *deleted_segment , {}, {}};

			{
				auto node_id = deleted_segment->m_firstNodeId;
				auto* node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && *segment == deleted_segment_id)
					{
						stored_segment.first_side = static_cast<model::ConnectedSegmentSide>(i);
						node->SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			{
				auto node_id = deleted_segment->m_secondNodeId;
				auto* node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && (*segment) == deleted_segment_id)
					{
						stored_segment.second_side = static_cast<model::ConnectedSegmentSide>(i);
						node->SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			scene.RemoveNetSegmentById(deleted_segment_id);
			edits.stored_deleted_segments.push_back(std::move(stored_segment));
		}

		// handle deleted nodes
		for (auto&& deleted_node : update_request.removed_nodes)
		{
			auto* node = scene.GetNetNodeById(deleted_node);
			assert(node);
			if (!node)
			{
				return false;
			}
			auto* conn = scene.GetSocketConnectionForNode(node->GetId());
			if (conn)
			{
				edits.stored_deleted_connections.push_back(*conn);
				scene.RemoveSocketConnectionForSocket(conn->socketId);
			}
			model::NetNodeModel stored_node = *node;
			auto* net = scene.GetNet(node->GetNetId());
			assert(net);
			net->removeNode(node->GetId());
			scene.RemoveNetNodeById(deleted_node);
			edits.stored_deleted_nodes.push_back(std::move(stored_node));
		}

		{
			// handle deleted nets
			for (const auto& net_id : update_request.removed_nets)
			{
				auto* net = scene.GetNet(net_id);
				assert(net);
				assert(net->GetNodes().size() == 0);
				edits.stored_deleted_nets.push_back(*net);
				scene.RemoveNetById(net_id);
			}
		}

		std::vector<model::NetId> new_nets;
		{
			// handle added nets
			model::NetId max_id{ 0 };
			if (update_request.added_nets.size())
			{
				max_id = GetMaxNetId(scene.GetNets());
			}
			for (auto&& net_added : update_request.added_nets)
			{
				max_id.value += 1;
				scene.AddNet(max_id, net_added.category);
				new_nets.push_back(max_id);
				edits.stored_new_nets.push_back(max_id);
			}
		}

		// handle added nodes
		std::vector<model::NetNodeId> new_nodes;
		new_nodes.reserve(update_request.added_nodes.size());
		model::NetNodeId max_id{ 0 };
		if (update_request.added_nodes.size())
		{
			max_id = GetMaxNodeId(scene.GetNetNodes());
		}
		for (auto&& node_request : update_request.added_nodes)
		{
			model::NetId net_id = node_request.net_type == NetModificationRequest::IdType::existing_id ? 
				node_request.net_id : edits.stored_new_nets[node_request.net_id.value];
			max_id.value += 1;
			scene.AddNetNode(model::NetNodeModel{ max_id,node_request.position, net_id });
			edits.stored_added_nodes.push_back(max_id);
			new_nodes.push_back(max_id);
			auto* net = scene.GetNet(net_id);
			assert(net);
			net->AddNode(max_id);
		}

		// handle updated Nodes
		for (auto&& node_request : update_request.update_nodes)
		{
			auto* node = scene.GetNetNodeById(node_request.node_id);
			assert(node);
			if (node)
			{
				model::Point old_position = node->GetPosition();
				node->SetPosition(node_request.new_position);
				edits.stored_node_updates.push_back(StoredNodeUpdates{ node_request.node_id, old_position });
			}
		}

		// handle node net change
		std::vector<NetModificationReport::UpdateNodeNetReport> node_net_changed;
		{
			for (const auto& node_modification : update_request.update_nodes_nets)
			{
				auto* node = scene.GetNetNodeById(node_modification.node);
				assert(node);
				auto old_net_id = node->GetNetId();
				edits.stored_node_net_change.push_back(StoredNodeNetChange{ .node = node_modification.node, .net = old_net_id });
				auto* old_net = scene.GetNet(old_net_id);
				old_net->removeNode(node_modification.node);
				if (node_modification.net_type == NetModificationRequest::IdType::existing_id)
				{
					auto* new_net = scene.GetNet(node_modification.net);
					assert(new_net);
					node->SetNetId(node_modification.net);
					new_net->AddNode(node_modification.node);
				}
				else // new_id
				{
					assert(node_modification.net.value < new_nets.size());
					auto new_net_id = new_nets[node_modification.net.value];
					auto* new_net = scene.GetNet(new_net_id);
					assert(new_net);
					node->SetNetId(new_net_id);
					new_net->AddNode(node_modification.node);
				}
				node_net_changed.push_back({ node_modification.node, node->GetNetId() });
			}
		}

		// handle net category change
		for (auto&& net_change : update_request.changed_net_categories)
		{
			auto* net = scene.GetNet(net_change.net_id);
			assert(net);
			edits.stored_net_category_change.push_back(StoredNetCategoryChange{ .net_id = net_change.net_id, .old_category = net->GetCategory() });
			net->SetCategory(net_change.new_category);
			for (auto& node_id : net->GetNodes())
			{
				node_net_changed.push_back({ node_id, net->GetId() });
			}
		}

		// handle merge nets
		for (auto&& merged_net : update_request.merged_nets)
		{
			auto* net1 = scene.GetNet(merged_net.original_net);
			auto* net2 = scene.GetNet(merged_net.merged_net);
			assert(net1);
			assert(net2);
			model::NetCategory new_category = net1->GetCategory();
			if (!net2->GetCategory().IsEmpty())
			{
				new_category = net2->GetCategory();
			}
			edits.stored_merged_net.push_back(StoredNetMerge{ 
				.net1_id = net1->GetId(), .net1_old_category = net1->GetCategory(), .second_net= *net2 });
			if (net1->GetCategory() != new_category)
			{
				net1->SetCategory(new_category);
				for (auto& node : net1->GetNodes())
				{
					node_net_changed.push_back({ node, net1->GetId() });
				}
			}
			for (auto& node : net2->GetNodes())
			{
				auto* node_ptr = scene.GetNetNodeById(node);
				assert(node_ptr);
				node_ptr->SetNetId(merged_net.original_net);
				net1->AddNode(node);
				node_net_changed.push_back({ node, net1->GetId() });
			}
			scene.RemoveNetById(merged_net.merged_net);

		}

		// handle updated segments
		for (auto&& segment_request : update_request.update_segments)
		{
			auto* updated_segment = scene.GetNetSegmentById(segment_request.segment_id);
			assert(updated_segment);
			if (!updated_segment)
			{
				return false;
			}

			StoredSegmentUpdate segment_update{ updated_segment->GetId(), model::NetNodeId{0}, model::NetNodeId{0} , {}, {}, {} };
			{
				// disconnect from old node
				auto node_id = updated_segment->m_firstNodeId;
				auto* node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && *segment == segment_request.segment_id)
					{
						segment_update.old_first_side = static_cast<model::ConnectedSegmentSide>(i);
						segment_update.old_node1 = node->GetId();
						node->SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			{
				// disconnect from old node
				auto node_id = updated_segment->m_secondNodeId;
				auto* node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && (*segment) == segment_request.segment_id)
					{
						segment_update.old_second_side = static_cast<model::ConnectedSegmentSide>(i);
						segment_update.old_node2 = node->GetId();
						node->SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			


			if (segment_request.node1_type == NetModificationRequest::IdType::existing_id)
			{
				updated_segment->m_firstNodeId = segment_request.node1;
			}
			else
			{
				assert(static_cast<size_t>(segment_request.node1.value) < new_nodes.size());
				if (static_cast<size_t>(segment_request.node1.value) < new_nodes.size())
				{
					updated_segment->m_firstNodeId = new_nodes[segment_request.node1.value];
				}
			}
			if (segment_request.node2_type == NetModificationRequest::IdType::existing_id)
			{
				updated_segment->m_secondNodeId = segment_request.node2;
			}
			else
			{
				assert(static_cast<size_t>(segment_request.node2.value) < new_nodes.size());
				if (static_cast<size_t>(segment_request.node2.value) < new_nodes.size())
				{
					updated_segment->m_secondNodeId = new_nodes[segment_request.node2.value];
				}
			}
			segment_update.old_orientation = updated_segment->m_orientation;
			updated_segment->m_orientation = segment_request.orientation;

			{
				auto node_id1 = updated_segment->m_firstNodeId;
				auto* node1 = scene.GetNetNodeById(node_id1);
				assert(node1);
				auto node_id2 = updated_segment->m_secondNodeId;
				auto* node2 = scene.GetNetNodeById(node_id2);
				assert(node2);

				if (!node1 || !node2)
				{
					return false;
				}

				node1->SetSegmentAt(segment_request.node1_side, updated_segment->GetId());
				node2->SetSegmentAt(segment_request.node2_side, updated_segment->GetId());
			}
			edits.stored_segment_updates.push_back(std::move(segment_update));
		}

		// handle added segments
		std::vector<model::NetSegmentId> new_segments;
		new_segments.reserve(update_request.added_segments.size());
		model::NetSegmentId max_segment_id{ 0 };
		if (update_request.added_segments.size())
		{
			max_segment_id = GetMaxSegmentId(scene.GetNetSegments());
		}
		for (auto&& segment_request : update_request.added_segments)
		{
			max_segment_id.value += 1;
			model::NetSegmentId id{ max_segment_id.value };
			std::optional<model::NetNodeId> node1_id;
			std::optional<model::NetNodeId> node2_id;
			if (segment_request.node1_type == NetModificationRequest::IdType::existing_id)
			{
				node1_id = segment_request.node1;
			}
			else
			{
				assert(static_cast<size_t>(segment_request.node1.value) < new_nodes.size());
				if (static_cast<size_t>(segment_request.node1.value) < new_nodes.size())
				{
					node1_id = new_nodes[segment_request.node1.value];
				}
			}
			if (segment_request.node2_type == NetModificationRequest::IdType::existing_id)
			{
				node2_id = segment_request.node2;
			}
			else
			{
				assert(static_cast<size_t>(segment_request.node2.value) < new_nodes.size());
				if (static_cast<size_t>(segment_request.node2.value) < new_nodes.size())
				{
					node2_id = new_nodes[segment_request.node2.value];
				}
			}

			if (!node1_id || !node2_id)
			{
				return false;
			}

			auto* node1 = scene.GetNetNodeById(*node1_id);
			assert(node1);
			auto* node2 = scene.GetNetNodeById(*node2_id);
			assert(node2);
			if (!node1 || !node2)
			{
				return false;
			}

			{
				model::NetSegmentModel new_segment{ id, *node1_id, *node2_id, segment_request.orientation };
				scene.AddNetSegment(std::move(new_segment));
				new_segments.push_back(id);
				edits.stored_new_segments.push_back(StoredAddedSegment{ id, segment_request.node1_side, segment_request.node2_side });
			}
			node1->SetSegmentAt(segment_request.node1_side, id);
			node2->SetSegmentAt(segment_request.node2_side, id);
		}

		// handle added connections
		for (auto&& connection : update_request.added_connections)
		{
			auto* block = scene.GetBlockById(connection.socket.block_id);
			assert(block);
			if (block)
			{
				auto node_id = connection.node;
				if (connection.node_type == NetModificationRequest::IdType::new_id)
				{
					assert(static_cast<size_t>(connection.node.value) < new_nodes.size());

					node_id = new_nodes[static_cast<size_t>(connection.node.value)];
				}
				auto* socket = block->GetSocketById(connection.socket.socket_id);
				assert(socket);
				socket->SetConnectedNetNode(node_id);
				scene.AddSocketNodeConnection(model::SocketNodeConnection{ connection.socket , node_id });
				edits.stored_new_connections.push_back({ connection.socket, node_id });
			}
		}


		// setup notification
		NetModificationReport report;
		report.removed_connections = update_request.removed_connections;
		report.removed_nodes = update_request.removed_nodes;
		report.removed_segments = update_request.removed_segments;
		report.node_net_changed = std::move(node_net_changed);

		report.added_nodes.reserve(update_request.added_nodes.size());
		for (auto&& node_id : new_nodes)
		{
			auto* node = scene.GetNetNodeById(node_id);
			assert(node);
			report.added_nodes.push_back(*node);
		}

		report.update_nodes.reserve(update_request.update_nodes.size());
		for (auto&& node_update : update_request.update_nodes)
		{
			report.update_nodes.push_back({ node_update.node_id, node_update.new_position });
		}

		report.update_segments.reserve(update_request.update_segments.size());
		for (auto&& update_segment_request : update_request.update_segments)
		{
			auto* segment = scene.GetNetSegmentById(update_segment_request.segment_id);
			assert(segment);
			report.update_segments.push_back(*segment);
		}

		report.added_segments.reserve(new_segments.size());
		for (auto&& added_segment : new_segments)
		{
			auto* segment = scene.GetNetSegmentById(added_segment);
			assert(segment);
			report.added_segments.push_back(*segment);
		}

		report.added_connections.reserve(update_request.added_connections.size());
		for (auto&& added_connection : update_request.added_connections)
		{
			auto node_id = added_connection.node;
			if (added_connection.node_type == NetModificationRequest::IdType::new_id)
			{
				assert(static_cast<size_t>(added_connection.node.value) < new_nodes.size());
				node_id = new_nodes[static_cast<size_t>(added_connection.node.value)];
			}
			auto* conn = scene.GetSocketConnectionForNode(node_id);
			assert(conn);
			report.added_connections.push_back(*conn);
		}

		manager.Notify(SceneModification{ SceneModification::type_t::NetUpdated, SceneModification::data_t{std::ref(report)} });

		return true;
	}
};


struct MoveBlockAction : public ModelAction
{
	MoveBlockAction(model::Point dst_point, model::BlockId id, NetModificationRequest&& update_request)
		:dst_point{ dst_point }, id{ id }, m_update_net_action{ std::move(update_request) } {}
	model::Point src_point{};
	const model::Point dst_point;
	const model::BlockId id;
	UpdateNetAction m_update_net_action;

	bool DoUndo(SceneModelManager& manager) override
	{
		if (!m_update_net_action.DoUndo(manager))
		{
			return false;
		}
		auto* block = manager.GetModel().GetBlockById(id);
		assert(block);
		if (!block)
		{
			return false;
		}

		block->SetPosition(src_point);
		manager.Notify(SceneModification{ SceneModificationType::BlockMoved, SceneModification::data_t{*block} });

		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		if (!m_update_net_action.DoRedo(manager))
		{
			return false;
		}
		auto* block = manager.GetModel().GetBlockById(id);
		assert(block);
		if (!block)
		{
			return false;
		}

		src_point = block->GetPosition();

		block->SetPosition(dst_point);
		manager.Notify(SceneModification{ SceneModificationType::BlockMoved, SceneModification::data_t{*block} });
		return true;
	}

};


struct ResizeBlockAction : public ModelAction
{
	ResizeBlockAction(model::Rect dst_rect, model::BlockId id, model::BlockOrientation new_orientation,
		std::vector<model::BlockSocketModel> new_sockets_positions, NetModificationRequest&& net_update)
		:dst_rect{ dst_rect }, id{ id }, new_orientation{ new_orientation },
		new_sockets_positions{ new_sockets_positions }, net_update_action{ std::move(net_update) } {
	}
	model::Rect src_rect{};
	std::vector<model::BlockSocketModel> old_sockets_positions;
	const model::Rect dst_rect;
	const model::BlockId id;
	const model::BlockOrientation new_orientation;
	model::BlockOrientation old_orientation = model::BlockOrientation::LeftToRight;
	const std::vector<model::BlockSocketModel> new_sockets_positions;
	UpdateNetAction net_update_action;

	bool DoUndo(SceneModelManager& manager) override
	{
		auto* block = manager.GetModel().GetBlockById(id);
		assert(block);
		if (!block)
		{
			return false;
		}

		if (!net_update_action.DoUndo(manager))
		{
			return false;
		}

		block->SetOrientation(old_orientation);
		block->SetBounds(src_rect);
		for (auto&& socket : block->GetSockets())
		{
			auto old_socket_it = std::find_if(old_sockets_positions.begin(), old_sockets_positions.end(),
				[&](const model::BlockSocketModel& old_socket) {return old_socket.GetId() == socket.GetId(); });
			assert(old_socket_it != old_sockets_positions.end());
			if (old_socket_it == old_sockets_positions.end())
			{
				return false;
			}
			auto&& old_socket_pos = old_socket_it->GetPosition();
			auto&& old_socket_side = old_socket_it->GetConnectionSide();
			socket.SetPosition(old_socket_pos);
			socket.SetConnectionSide(old_socket_side);
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockResized, SceneModification::data_t{*block} });
		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		auto* block = manager.GetModel().GetBlockById(id);
		assert(block);
		if (!block)
		{
			return false;
		}

		if (!net_update_action.DoRedo(manager))
		{
			return false;
		}
		src_rect = block->GetBounds();
		old_orientation = block->GetOrienation();

		block->SetOrientation(new_orientation);
		block->SetBounds(dst_rect);
		for (auto&& socket : block->GetSockets())
		{
			auto new_socket_it = std::find_if(new_sockets_positions.begin(), new_sockets_positions.end(), [&](const model::BlockSocketModel& new_socket) {return new_socket.GetId() == socket.GetId(); });
			assert(new_socket_it != new_sockets_positions.end());
			if (new_socket_it == new_sockets_positions.end())
			{
				return false;
			}
			auto&& old_socket_pos = socket.GetPosition();
			auto&& old_socket_side = socket.GetConnectionSide();
			old_sockets_positions.push_back(model::BlockSocketModel{ socket.GetType(), socket.GetId(), old_socket_pos, old_socket_side });
			auto&& new_socket_pos = new_socket_it->GetPosition();
			auto&& new_socket_side = new_socket_it->GetConnectionSide();
			socket.SetPosition(new_socket_pos);
			socket.SetConnectionSide(new_socket_side);
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockResized, SceneModification::data_t{*block} });
		return true;
	}

};

}

node::SceneModelManager::SceneModelManager(std::shared_ptr<model::NodeSceneModel> scene)
	:m_scene(std::move(scene))
{
	assert(m_scene);
}

node::SceneModelManager::~SceneModelManager()
{
}

std::span<node::model::BlockModel> node::SceneModelManager::GetBlocks()
{
	return m_scene->GetBlocks();
}

void node::SceneModelManager::AddNewSubsystemBlock(model::BlockModel&& block, model::SubsystemBlockData&& data)
{
	auto action = std::make_unique<AddSubsystemBlockAction>(std::move(block), std::move(data));
	PushAction(std::move(action));
}

void node::SceneModelManager::AddNewFunctionalBlock(model::BlockModel&& block, model::FunctionalBlockData&& data)
{
	auto action = std::make_unique<AddFuncitonalBlockAction>(std::move(block), std::move(data));
	PushAction(std::move(action));
}

void node::SceneModelManager::AddNewPortBlock(model::BlockModel&& block, model::PortBlockData&& data)
{
	auto action = std::make_unique<AddPortBlockAction>(std::move(block), std::move(data));
	PushAction(std::move(action));
}

void node::SceneModelManager::RemoveBlockById(const model::BlockId& id)
{
	auto action = std::make_unique<RemoveBlockAction>(id);
	PushAction(std::move(action));
}

void node::SceneModelManager::ResizeBlockById(const model::BlockId& id, 
	const model::Rect& new_rect, model::BlockOrientation new_orientation, 
	std::vector<model::BlockSocketModel> socket_positions,
	NetModificationRequest&& net_update)
{
	auto action = std::make_unique<ResizeBlockAction>(new_rect, id, new_orientation, 
		std::move(socket_positions), std::move(net_update));
	PushAction(std::move(action));
}

void node::SceneModelManager::MoveBlockById(const model::BlockId& id, const model::Point& new_origin, NetModificationRequest&& net_update)
{
	auto move_action = std::make_unique<MoveBlockAction>(new_origin, id, std::move(net_update));
	PushAction(std::move(move_action));
}

void node::SceneModelManager::ModifyBlockProperties(model::BlockId id, std::vector<model::BlockProperty> new_properties)
{
	auto action = std::make_unique<ModifyBlockPropertiesAction>(id, std::move(new_properties));
	PushAction(std::move(action));
}

void node::SceneModelManager::ModifyBlockPropertiesAndSockets(model::BlockId id, std::vector<model::BlockProperty> new_properties, std::vector<model::BlockSocketModel> new_sockets)
{
	auto action = std::make_unique<ModifyBlockPropertiesAndSocketsAction>(id, std::move(new_properties), std::move(new_sockets));
	PushAction(std::move(action));
}

void node::SceneModelManager::ModifyBlockSockets(model::BlockId id, std::vector<model::BlockSocketModel> new_sockets)
{
	auto action = std::make_unique<ModifyBlockSocketsAction>(id, std::move(new_sockets));
	PushAction(std::move(action));
}

void node::SceneModelManager::UpdateNet(NetModificationRequest&& update_request)
{
	auto action = std::make_unique<UpdateNetAction>(std::move(update_request));
	PushAction(std::move(action));
}

void node::SceneModelManager::Undo()
{
	assert(m_undo_stack.size());
	if (!m_undo_stack.size())
	{
		return;
	}

	auto& action = m_undo_stack.top();
	if (action->Undo(*this))
	{
		m_redo_stack.push(std::move(action));
		m_undo_stack.pop();
	}
	
}

void node::SceneModelManager::Redo()
{
	assert(m_redo_stack.size());
	if (!m_redo_stack.size())
	{
		return;
	}

	auto& action = m_redo_stack.top();
	if (action->Redo(*this))
	{
		m_undo_stack.push(std::move(action));
		m_redo_stack.pop();
	}
		
}

void node::SceneModelManager::PushAction(std::unique_ptr<ModelAction> action)
{
	if (action->Redo(*this))
	{
		m_undo_stack.push(std::move(action));
		while (m_redo_stack.size()) { m_redo_stack.pop(); }
	}
}
