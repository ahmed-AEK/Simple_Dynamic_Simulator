#include "SceneModelManager.hpp"

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

void node::SceneModelManager::AddNewBlock(model::BlockModel&& block)
{
	model::id_int max_id = 0;
	for (auto&& it_block : m_scene->GetBlocks())
	{
		max_id = std::max(max_id, it_block.GetId().value);
	}
	model::BlockId block_id{ max_id + 1 };
	block.SetId(block_id);
	m_scene->AddBlock(std::move(block));

	auto block_ref = m_scene->GetBlockById(block_id);
	assert(block_ref);
	Notify(SceneModification{ SceneModificationType::BlockAdded, SceneModification::data_t{*block_ref} });
}

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

void node::SceneModelManager::RemoveBlockById(const model::BlockId& id)
{
	auto block = m_scene->GetBlockById(id);
	assert(block);
	if (block)
	{
		for (const auto& socket : block->get().GetSockets())
		{
			auto connected_node = socket.GetConnectedNetNode();
			if (connected_node)
			{
				m_scene->RemoveSocketConnectionForSocket({ socket.GetId(), block->get().GetId() });
			}
		}
		m_scene->RemoveBlockById(id);
		Notify(SceneModification{ SceneModificationType::BlockRemoved, SceneModification::data_t{id} });
	}
}

static void MoveNodeAndConnectedNodes(const node::model::NetNodeId& node_Id, const node::model::Point& point, 
	node::model::NodeSceneModel& model)
{
	using namespace node::model;
;
	auto main_node_wrap = model.GetNetNodeById(node_Id);
	assert(main_node_wrap);
	if (!main_node_wrap)
	{
		return;
	}

	NetNodeModel& main_node = (*main_node_wrap);
	main_node.SetPosition(point);
	
	auto correct_segment_at = [&](ConnectedSegmentSide side) {
		auto east_segment = main_node.GetSegmentAt(side);
		std::optional<NetNodeId> other_side{ std::nullopt };
		if (east_segment)
		{
			auto segment_model = model.GetNetSegmentById(*east_segment);
			if (segment_model)
			{
				other_side = segment_model->get().m_firstNodeId;
				if (other_side == node_Id)
				{
					other_side = segment_model->get().m_secondNodeId;
				}
			}
		}
		if (other_side)
		{
			auto other_node = model.GetNetNodeById(*other_side);
			if (other_node)
			{
				other_node->get().SetPosition({ other_node->get().GetPosition().x, point.y });
			}
		}
		};

	correct_segment_at(ConnectedSegmentSide::east);
	correct_segment_at(ConnectedSegmentSide::west);
	assert(!main_node.GetSegmentAt(ConnectedSegmentSide::north)); // TODO
	assert(!main_node.GetSegmentAt(ConnectedSegmentSide::south)); // TODO
}

void node::SceneModelManager::MoveBlockById(const model::BlockId& id, const model::Point& new_origin)
{
	auto block = m_scene->GetBlockById(id);
	assert(block);
	if (block)
	{
		block->get().SetPosition(new_origin);
		for (auto&& socket : block->get().GetSockets())
		{
			const auto& connected_node = socket.GetConnectedNetNode();
			if (connected_node)
			{
				MoveNodeAndConnectedNodes(*connected_node, new_origin, *m_scene);
			}
		}
		Notify(SceneModification{ SceneModificationType::BlockMoved, SceneModification::data_t{*block} });
	}
}

void node::SceneModelManager::ModifyBlockProperties(model::BlockId id, std::vector<model::BlockProperty> new_properties)
{
	auto block = m_scene->GetBlockById(id);
	assert(block);
	if (!block)
	{
		return;
	}

	block->get().GetProperties() = std::move(new_properties);
	Notify(SceneModification{ SceneModificationType::BlockPropertiesModified , SceneModification::data_t{*block} });
}

void node::SceneModelManager::ModifyBlockPropertiesAndSockets(model::BlockId id, std::vector<model::BlockProperty> new_properties, std::vector<model::BlockSocketModel> new_sockets)
{
	auto block = m_scene->GetBlockById(id);
	assert(block);
	if (!block)
	{
		return;
	}
	block->get().GetProperties() = std::move(new_properties);
	for (const auto& socket : block->get().GetSockets())
	{
		if (auto node_id = socket.GetConnectedNetNode())
		{
			m_scene->RemoveSocketConnectionForSocket({ socket.GetId(), block->get().GetId() });
		}
	}
	block->get().ClearSockets();
	block->get().ReserveSockets(new_sockets.size());
	for (auto&& socket : new_sockets)
	{
		block->get().AddSocket(std::move(socket));
	}
	Notify(SceneModification{ SceneModificationType::BlockPropertiesAndSocketsModified , SceneModification::data_t{*block} });
}

void node::SceneModelManager::UpdateNet(NetModificationRequest& update_request)
{

	// handle deleted connections
	for (const auto& deleted_connection : update_request.removed_connections)
	{
		auto block = m_scene->GetBlockById(deleted_connection.block_id);
		assert(block);
		if (block)
		{
			auto socket = block->get().GetSocketById(deleted_connection.socket_id);
			assert(socket);
			socket->get().SetConnectedNetNode(std::nullopt);
		}
		m_scene->RemoveSocketConnectionForSocket(deleted_connection);
	}

	// handle deleted segments
	for (auto&& deleted_segment_id : update_request.removed_segments)
	{
		auto deleted_segment = m_scene->GetNetSegmentById(deleted_segment_id);
		assert(deleted_segment);
		if (deleted_segment)
		{ 
			{
				auto node_id = deleted_segment->get().m_firstNodeId;
				auto node = m_scene->GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->get().GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && *segment == deleted_segment_id)
					{
						node->get().SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			{
				auto node_id = deleted_segment->get().m_secondNodeId;
				auto node = m_scene->GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->get().GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && (*segment) == deleted_segment_id)
					{
						node->get().SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			m_scene->RemoveNetSegmentById(deleted_segment_id);
		}
	}

	// handle deleted nodes
	for (auto&& deleted_node : update_request.removed_nodes)
	{
		auto node = m_scene->GetNetNodeById(deleted_node);
		assert(node);
		if (node)
		{
			auto conn = m_scene->GetSocketConnectionForNode(node->get().GetId());
			if (conn)
			{
				m_scene->RemoveSocketConnectionForSocket(conn->get().socketId);
			}
			m_scene->RemoveNetNodeById(deleted_node);
		}
	}

	// handle added nodes
	std::vector<model::NetNodeId> new_nodes;
	new_nodes.reserve(update_request.added_nodes.size());
	model::NetNodeId max_id{ 0 };
	if (update_request.added_nodes.size())
	{
		max_id = GetMaxNodeId(m_scene->GetNetNodes());
	}
	for (auto&& node_request : update_request.added_nodes)
	{
		max_id.value += 1;
		m_scene->AddNetNode(model::NetNodeModel{ max_id,node_request.position });
		new_nodes.push_back(max_id);
	}

	// handle updated Nodes
	for (auto&& node_request : update_request.update_nodes)
	{
		auto node = m_scene->GetNetNodeById(node_request.node_id);
		assert(node);
		if (node)
		{
			node->get().SetPosition(node_request.new_position);
		}
	}

	// handle updated segments
	for (auto&& segment_request : update_request.update_segments)
	{
		auto deleted_segment = m_scene->GetNetSegmentById(segment_request.segment_id);
		assert(deleted_segment);
		if (deleted_segment)
		{
			{
				auto node_id = deleted_segment->get().m_firstNodeId;
				auto node = m_scene->GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->get().GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && *segment == segment_request.segment_id)
					{
						node->get().SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			{
				auto node_id = deleted_segment->get().m_secondNodeId;
				auto node = m_scene->GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->get().GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && (*segment) == segment_request.segment_id)
					{
						node->get().SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
		}

		if (segment_request.node1_type == NetModificationRequest::NodeIdType::existing_id)
		{
			deleted_segment->get().m_firstNodeId = segment_request.node1;
		}
		else
		{
			assert(static_cast<size_t>(segment_request.node1.value) < new_nodes.size());
			if (static_cast<size_t>(segment_request.node1.value) < new_nodes.size())
			{
				deleted_segment->get().m_firstNodeId = new_nodes[segment_request.node1.value];
			}
		}
		if (segment_request.node2_type == NetModificationRequest::NodeIdType::existing_id)
		{
			deleted_segment->get().m_secondNodeId = segment_request.node2;
		}
		else
		{
			assert(static_cast<size_t>(segment_request.node2.value) < new_nodes.size());
			if (static_cast<size_t>(segment_request.node2.value) < new_nodes.size())
			{
				deleted_segment->get().m_secondNodeId = new_nodes[segment_request.node2.value];
			}
		}
		
		{
			auto node_id = deleted_segment->get().m_firstNodeId;
			auto node = m_scene->GetNetNodeById(node_id);
			assert(node);
			if (node)
			{
				node->get().SetSegmentAt(segment_request.node1_side, deleted_segment->get().GetId());
			}
		}
		{
			auto node_id = deleted_segment->get().m_secondNodeId;
			auto node = m_scene->GetNetNodeById(node_id);
			assert(node);
			if (node)
			{
				node->get().SetSegmentAt(segment_request.node2_side, deleted_segment->get().GetId());
			}
		}

	}

	// handle added segments
	std::vector<model::NetSegmentId> new_segments;
	new_segments.reserve(update_request.added_segments.size());
	model::NetSegmentId max_segment_id{ 0 };
	if (update_request.added_segments.size())
	{
		max_segment_id = GetMaxSegmentId(m_scene->GetNetSegments());
	}
	for (auto&& segment_request : update_request.added_segments)
	{
		max_segment_id.value += 1;
		model::NetSegmentId id{max_segment_id.value};
		std::optional<model::NetNodeId> node1_id;
		std::optional<model::NetNodeId> node2_id;
		if (segment_request.node1_type == NetModificationRequest::NodeIdType::existing_id)
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
		if (segment_request.node2_type == NetModificationRequest::NodeIdType::existing_id)
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

		assert(node1_id);
		assert(node2_id);
		if (!node1_id || !node2_id)
		{
			return;
		}

		{
			model::NetSegmentModel new_segment{ id, *node1_id, *node2_id, segment_request.orientation };
			m_scene->AddNetSegment(std::move(new_segment));
			new_segments.push_back(id);
		}

		{
			auto node = m_scene->GetNetNodeById(*node1_id);
			assert(node);
			if (node)
			{
				node->get().SetSegmentAt(segment_request.node1_side, id);
			}
		}
		{
			auto node = m_scene->GetNetNodeById(*node2_id);
			assert(node);
			if (node)
			{
				node->get().SetSegmentAt(segment_request.node2_side, id);
			}
		}
	}
	
	// handle added connections
	for (auto&& connection : update_request.added_connections)
	{
		auto block = m_scene->GetBlockById(connection.socket.block_id);
		assert(block);
		if (block)
		{
			auto node_id = connection.node;
			if (connection.node_type == NetModificationRequest::NodeIdType::new_id)
			{
				assert(static_cast<size_t>(connection.node.value) < new_nodes.size());

				node_id = new_nodes[static_cast<size_t>(connection.node.value)];
			}
			auto socket = block->get().GetSocketById(connection.socket.socket_id);
			assert(socket);
			socket->get().SetConnectedNetNode(node_id);
			m_scene->AddSocketNodeConnection(model::SocketNodeConnection{ connection.socket , node_id });
		}
	}


	// setup notification
	NetModificationReport report;
	report.removed_connections = std::move(update_request.removed_connections);
	report.removed_nodes = std::move(update_request.removed_nodes);
	report.removed_segments = std::move(update_request.removed_segments);
	report.added_nodes.reserve(update_request.added_nodes.size());
	for (auto&& node_id : new_nodes)
	{
		auto node = m_scene->GetNetNodeById(node_id);
		assert(node);
		report.added_nodes.push_back(*node);
	}
	for (auto&& node_update : update_request.update_nodes)
	{
		report.update_nodes.push_back({ node_update.node_id, node_update.new_position });
	}
	for (auto&& update_segment_request : update_request.update_segments)
	{
		auto segment = m_scene->GetNetSegmentById(update_segment_request.segment_id);
		assert(segment);
		report.update_segments.push_back(*segment);
	}
	for (auto&& added_segment : new_segments)
	{
		auto segment = m_scene->GetNetSegmentById(added_segment);
		assert(segment);
		report.added_segments.push_back(*segment);
	}
	for (auto&& added_connection : update_request.added_connections)
	{
		auto node_id = added_connection.node;
		if (added_connection.node_type == NetModificationRequest::NodeIdType::new_id)
		{
			assert(static_cast<size_t>(added_connection.node.value) < new_nodes.size());
			node_id = new_nodes[static_cast<size_t>(added_connection.node.value)];
		}
		auto conn = m_scene->GetSocketConnectionForNode(node_id);
		assert(conn);
		report.added_connections.push_back(*conn);
	}

	Notify(SceneModification{ SceneModification::type_t::NetUpdated, SceneModification::data_t{std::ref(report)} });

}
