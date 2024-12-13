#include "SceneModelManager.hpp"
#include <iterator>

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

namespace node
{
struct MoveBlockAction : public ModelAction
{
	MoveBlockAction(model::Point dst_point, model::BlockId id)
		:dst_point{ dst_point }, id{ id } {}
	model::Point src_point{};
	const model::Point dst_point;
	const model::BlockId id;
	bool DoUndo(SceneModelManager& manager) override
	{
		auto block = manager.GetModel().GetBlockById(id);
		assert(block);
		if (!block)
		{
			return false;
		}

		block->get().SetPosition(src_point);
		for (auto&& socket : block->get().GetSockets())
		{
			const auto& connected_node = socket.GetConnectedNetNode();
			if (connected_node)
			{
				MoveNodeAndConnectedNodes(*connected_node, 
					{ src_point.x + socket.GetPosition().x, src_point.y + socket.GetPosition().y}, manager.GetModel());
			}
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockMoved, SceneModification::data_t{*block} });
		
		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		auto block = manager.GetModel().GetBlockById(id);
		assert(block);
		if (!block)
		{
			return false;
		}

		src_point = block->get().GetPosition();

		block->get().SetPosition(dst_point);
		for (auto&& socket : block->get().GetSockets())
		{
			const auto& connected_node = socket.GetConnectedNetNode();
			auto&& socket_pos = socket.GetPosition();
			if (connected_node)
			{
				MoveNodeAndConnectedNodes(*connected_node, 
					{ dst_point.x + socket_pos.x, dst_point.y + socket_pos.y }, manager.GetModel());
			}
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockMoved, SceneModification::data_t{*block} });
		return true;
	}

};

struct ResizeBlockAction : public ModelAction
{
	ResizeBlockAction(model::Rect dst_rect, model::BlockId id, model::BlockOrientation new_orientation, std::vector<model::BlockSocketModel> new_sockets_positions)
		:dst_rect{ dst_rect }, id{ id }, new_orientation{ new_orientation }, 
		new_sockets_positions{new_sockets_positions} {}
	model::Rect src_rect{};
	std::vector<model::BlockSocketModel> old_sockets_positions;
	const model::Rect dst_rect;
	const model::BlockId id;
	const model::BlockOrientation new_orientation;
	model::BlockOrientation old_orientation = model::BlockOrientation::LeftToRight;
	const std::vector<model::BlockSocketModel> new_sockets_positions;

	bool DoUndo(SceneModelManager& manager) override
	{
		auto block = manager.GetModel().GetBlockById(id);
		assert(block);
		if (!block)
		{
			return false;
		}

		block->get().SetOrientation(old_orientation);
		block->get().SetBounds(src_rect);
		for (auto&& socket : block->get().GetSockets())
		{
			auto old_socket_it = std::find_if(old_sockets_positions.begin(), old_sockets_positions.end(), 
				[&](const model::BlockSocketModel& old_socket) {return old_socket.GetId() == socket.GetId(); });
			assert(old_socket_it != old_sockets_positions.end());
			if (old_socket_it == old_sockets_positions.end())
			{
				return false;
			}
			const auto& connected_node = socket.GetConnectedNetNode();
			auto&& old_socket_pos = old_socket_it->GetPosition();
			socket.SetPosition(old_socket_pos);
			if (connected_node)
			{
				MoveNodeAndConnectedNodes(*connected_node,
					{ dst_rect.x + old_socket_pos.x, dst_rect.y + old_socket_pos.y }, manager.GetModel());
			}
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockResized, SceneModification::data_t{*block} });
		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		auto block = manager.GetModel().GetBlockById(id);
		assert(block);
		if (!block)
		{
			return false;
		}

		src_rect = block->get().GetBounds();
		old_orientation = block->get().GetOrienation();

		block->get().SetOrientation(new_orientation);
		block->get().SetBounds(dst_rect);
		for (auto&& socket : block->get().GetSockets())
		{
			auto new_socket_it = std::find_if(new_sockets_positions.begin(), new_sockets_positions.end(), [&](const model::BlockSocketModel& new_socket) {return new_socket.GetId() == socket.GetId(); });
			assert(new_socket_it != new_sockets_positions.end());
			if (new_socket_it == new_sockets_positions.end())
			{
				return false;
			}
			const auto& connected_node = socket.GetConnectedNetNode();
			auto&& old_socket_pos = socket.GetPosition();
			old_sockets_positions.push_back(model::BlockSocketModel{ socket.GetType(), socket.GetId(), old_socket_pos});
			auto&& new_socket_pos = new_socket_it->GetPosition();
			socket.SetPosition(new_socket_pos);
			if (connected_node)
			{
				MoveNodeAndConnectedNodes(*connected_node,
					{ dst_rect.x + new_socket_pos.x, dst_rect.y + new_socket_pos.y }, manager.GetModel());
			}
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockResized, SceneModification::data_t{*block} });
		return true;
	}

};


struct AddBlockAction : public ModelAction
{
	explicit AddBlockAction(model::BlockModel block)
		:block{ block } {}
	const model::BlockModel block;
	std::optional<model::BlockId> stored_id;

	bool DoUndo(SceneModelManager& manager) override
	{
		assert(stored_id);
		if (!stored_id)
		{
			return false;
		}

		auto block_ref = manager.GetModel().GetBlockById(*stored_id);
		assert(block_ref);
		if (!block_ref)
		{
			return false;
		}
		manager.GetModel().RemoveBlockById(*stored_id);
		manager.UnRegisterSubSystem(*stored_id);
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

		manager.GetModel().AddBlock(std::move(temp_block));

		auto block_ref = manager.GetModel().GetBlockById(block_id);
		assert(block_ref);
		if (block_ref->get().GetClass() == "SubSystem")
		{
			manager.RegisterSubSystem(block_ref->get());
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockAdded, SceneModification::data_t{*block_ref} });
		
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

	bool DoUndo(SceneModelManager& manager) override
	{
		assert(stored_block);
		manager.GetModel().AddBlock(model::BlockModel{ *stored_block });

		auto block_ref = manager.GetModel().GetBlockById(stored_block->GetId());
		assert(block_ref);
		if (!block_ref)
		{
			return false;
		}

		for (auto&& connection : stored_connections)
		{
			manager.GetModel().AddSocketNodeConnection(connection);
		}
		if (block_ref->get().GetClass() == "SubSystem")
		{
			manager.RegisterSubSystem(block_ref->get());
		}
		manager.Notify(SceneModification{ SceneModificationType::BlockAddedWithConnections, SceneModification::data_t{BlockAddWithConnectionsReport{*block_ref,stored_connections} } });

		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		auto block = manager.GetModel().GetBlockById(id);
		assert(block);
		if (!block)
		{
			return false;
		}

		stored_connections.clear();
		for (const auto& socket : block->get().GetSockets())
		{
			auto connected_node = socket.GetConnectedNetNode();
			if (connected_node)
			{
				stored_connections.push_back(*manager.GetModel().GetSocketConnectionForNode(*connected_node));
				manager.GetModel().RemoveSocketConnectionForSocket({ socket.GetId(), block->get().GetId() });
			}
		}
		stored_block = block;
		manager.GetModel().RemoveBlockById(id);
		manager.UnRegisterSubSystem(id);
		manager.Notify(SceneModification{ SceneModificationType::BlockRemoved, SceneModification::data_t{id} });
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
		auto block = manager.GetModel().GetBlockById(block_id);
		assert(block);
		if (!block)
		{
			return false;
		}

		block->get().GetProperties() = old_properties;
		manager.Notify(SceneModification{ SceneModificationType::BlockPropertiesModified , SceneModification::data_t{*block} });
		
		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		auto block = manager.GetModel().GetBlockById(block_id);
		assert(block);
		if (!block)
		{
			return false;
		}
		old_properties = block->get().GetProperties();
		block->get().GetProperties() = new_properties;
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
		auto block = manager.GetModel().GetBlockById(block_id);
		assert(block);
		if (!block)
		{
			return false;
		}
		block->get().GetProperties() = old_properties;

		for (auto&& connection : stored_connections)
		{
			manager.GetModel().AddSocketNodeConnection(connection);
		}

		block->get().ClearSockets();
		block->get().ReserveSockets(old_sockets.size());
		for (auto&& socket : old_sockets)
		{
			block->get().AddSocket(std::move(socket));
		}

		manager.Notify(SceneModification{ SceneModificationType::BlockPropertiesAndSocketsModified , SceneModification::data_t{*block} });

		return true;
	}

	bool DoRedo(SceneModelManager& manager) override
	{
		auto block = manager.GetModel().GetBlockById(block_id);
		assert(block);
		if (!block)
		{
			return false;
		}

		old_properties = block->get().GetProperties();
		block->get().GetProperties() = new_properties;

		for (const auto& socket : block->get().GetSockets())
		{
			if (auto node_id = socket.GetConnectedNetNode())
			{
				stored_connections.push_back(*manager.GetModel().GetSocketConnectionForNode(*node_id));
				manager.GetModel().RemoveSocketConnectionForSocket({ socket.GetId(), block->get().GetId() });
			}
		}
		auto&& old_span = block->get().GetSockets();

		old_sockets.clear();
		old_sockets.reserve(old_span.size());
		std::copy(old_span.begin(), old_span.end(), std::back_inserter(old_sockets));

		block->get().ClearSockets();
		block->get().ReserveSockets(new_sockets.size());
		for (auto&& socket : new_sockets)
		{
			block->get().AddSocket(socket);
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
	};

	struct StoredAddedSegment
	{
		model::NetSegmentId id;
		model::ConnectedSegmentSide first_side;
		model::ConnectedSegmentSide second_side;
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
	};

	StoredEdits edits;

	bool DoUndo(SceneModelManager& manager) override
	{
		auto&& scene = manager.GetModel();
		// remove added connections
		for (auto&& connection : edits.stored_new_connections)
		{
			auto block = scene.GetBlockById(connection.socketId.block_id);
			assert(block);
			if (!block)
			{
				return false;
			}
			auto socket = block->get().GetSocketById(connection.socketId.socket_id);
			assert(socket);
			socket->get().SetConnectedNetNode(std::nullopt);
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

			auto node1 = scene.GetNetNodeById(segment->get().m_firstNodeId);
			assert(node1);
			auto node2 = scene.GetNetNodeById(segment->get().m_secondNodeId);
			assert(node2);
			if (!node1 || !node2)
			{
				return false;
			}
			scene.RemoveNetSegmentById(segment_info.id);
			node1->get().SetSegmentAt(segment_info.first_side,std::nullopt);
			node2->get().SetSegmentAt(segment_info.second_side, std::nullopt);
		}

		// undo updated segments
		for (auto&& segment_request : edits.stored_segment_updates)
		{
			auto updated_segment = scene.GetNetSegmentById(segment_request.id);
			assert(updated_segment);
			if (!updated_segment)
			{
				return false;
			}

			{
				// disconnect from old node
				auto node_id = updated_segment->get().m_firstNodeId;
				auto node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->get().GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && *segment == segment_request.id)
					{
						node->get().SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			{
				// disconnect from old node
				auto node_id = updated_segment->get().m_secondNodeId;
				auto node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->get().GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && (*segment) == segment_request.id)
					{
						node->get().SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}

			{
				auto node_id1 = segment_request.old_node1;
				auto node1 = scene.GetNetNodeById(node_id1);
				assert(node1);
				auto node_id2 = segment_request.old_node2;
				auto node2 = scene.GetNetNodeById(node_id2);
				assert(node2);

				if (!node1 || !node2)
				{
					return false;
				}

				updated_segment->get().m_firstNodeId = segment_request.old_node1;
				updated_segment->get().m_secondNodeId = segment_request.old_node2;

				node1->get().SetSegmentAt(segment_request.old_first_side, updated_segment->get().GetId());
				node2->get().SetSegmentAt(segment_request.old_second_side, updated_segment->get().GetId());
			}
		}

		// undo updated Nodes
		for (auto&& node_request : edits.stored_node_updates)
		{
			auto node = scene.GetNetNodeById(node_request.id);
			assert(node);
			if (node)
			{
				node->get().SetPosition(node_request.old_position);
			}
		}

		// undo added nodes
		for (auto&& node_id : edits.stored_added_nodes)
		{
			scene.RemoveNetNodeById(node_id);
		}

		// undo deleted nodes
		for (auto&& deleted_node : edits.stored_deleted_nodes)
		{
			scene.AddNetNode(model::NetNodeModel{ deleted_node });
		}

		// undo deleted segments
		for (auto&& deleted_segment_report : edits.stored_deleted_segments)
		{
			
			scene.AddNetSegment(model::NetSegmentModel{ deleted_segment_report.segment });
			{
				auto node_id = deleted_segment_report.segment.m_firstNodeId;
				auto node = scene.GetNetNodeById(node_id);
				assert(node);
				if (!node)
				{
					return false;
				}
				node->get().SetSegmentAt(deleted_segment_report.first_side, deleted_segment_report.segment.GetId());
			}
			
			{
				auto node_id = deleted_segment_report.segment.m_secondNodeId;
				auto node = scene.GetNetNodeById(node_id);
				assert(node);
				if (!node)
				{
					return false;
				}
				node->get().SetSegmentAt(deleted_segment_report.second_side, deleted_segment_report.segment.GetId());
			}
		}

		// undo deleted connections
		for (const auto& deleted_connection_report : edits.stored_deleted_connections)
		{
			auto block = scene.GetBlockById(deleted_connection_report.socketId.block_id);
			assert(block);
			if (!block)
			{
				return false;
			}
			auto socket = block->get().GetSocketById(deleted_connection_report.socketId.socket_id);
			assert(socket);
			if (!socket)
			{
				return false;
			}
			scene.AddSocketNodeConnection(model::SocketNodeConnection{deleted_connection_report.socketId, deleted_connection_report.NodeId});
			socket->get().SetConnectedNetNode(deleted_connection_report.NodeId);
		}
		
		// setup notification
		NetModificationReport report;
		std::transform(edits.stored_new_connections.begin(), edits.stored_new_connections.end(),
			std::back_inserter(report.removed_connections),
			[](const model::SocketNodeConnection& conn) {return conn.socketId; });

		std::transform(edits.stored_new_segments.begin(), edits.stored_new_segments.end(),
			std::back_inserter(report.removed_segments),
			[](const StoredAddedSegment& segment)->model::NetSegmentId {return segment.id; });

		report.removed_nodes = edits.stored_added_nodes;

		report.added_nodes.reserve(edits.stored_deleted_nodes.size());
		for (auto&& added_node : edits.stored_deleted_nodes)
		{
			auto node = scene.GetNetNodeById(added_node.GetId());
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
			auto segment = scene.GetNetSegmentById(update_segment_request.segment_id);
			assert(segment);
			report.update_segments.push_back(*segment);
		}

		report.added_segments.reserve(edits.stored_deleted_segments.size());
		for (auto&& added_segment : edits.stored_deleted_segments)
		{
			auto segment = scene.GetNetSegmentById(added_segment.segment.GetId());
			assert(segment);
			report.added_segments.push_back(*segment);
		}

		report.added_connections.reserve(edits.stored_deleted_connections.size());
		for (auto&& added_connection : edits.stored_deleted_connections)
		{
			auto&& node_id = added_connection.NodeId;
			auto conn = scene.GetSocketConnectionForNode(node_id);
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

		auto&& scene = manager.GetModel();
		// handle deleted connections
		for (const auto& deleted_connection : update_request.removed_connections)
		{
			auto&& conn = scene.GetSocketConnectionForSocket(deleted_connection);
			auto block = scene.GetBlockById(deleted_connection.block_id);
			assert(conn);
			assert(block);
			if (!conn || !block)
			{
				return false;
			}
			auto socket = block->get().GetSocketById(deleted_connection.socket_id);
			assert(socket);
			socket->get().SetConnectedNetNode(std::nullopt);
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
				auto node_id = deleted_segment->get().m_firstNodeId;
				auto node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->get().GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && *segment == deleted_segment_id)
					{
						stored_segment.first_side = static_cast<model::ConnectedSegmentSide>(i);
						node->get().SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			{
				auto node_id = deleted_segment->get().m_secondNodeId;
				auto node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->get().GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && (*segment) == deleted_segment_id)
					{
						stored_segment.second_side = static_cast<model::ConnectedSegmentSide>(i);
						node->get().SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
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
			auto node = scene.GetNetNodeById(deleted_node);
			assert(node);
			if (!node)
			{
				return false;
			}
			auto conn = scene.GetSocketConnectionForNode(node->get().GetId());
			if (conn)
			{
				edits.stored_deleted_connections.push_back(*conn);
				scene.RemoveSocketConnectionForSocket(conn->get().socketId);
			}
			model::NetNodeModel stored_node = node->get();
			scene.RemoveNetNodeById(deleted_node);
			edits.stored_deleted_nodes.push_back(std::move(stored_node));
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
			max_id.value += 1;
			scene.AddNetNode(model::NetNodeModel{ max_id,node_request.position });
			edits.stored_added_nodes.push_back(max_id);
			new_nodes.push_back(max_id);
		}

		// handle updated Nodes
		for (auto&& node_request : update_request.update_nodes)
		{
			auto node = scene.GetNetNodeById(node_request.node_id);
			assert(node);
			if (node)
			{
				model::Point old_position = node->get().GetPosition();
				node->get().SetPosition(node_request.new_position);
				edits.stored_node_updates.push_back(StoredNodeUpdates{ node_request.node_id, old_position });
			}
		}

		// handle updated segments
		for (auto&& segment_request : update_request.update_segments)
		{
			auto updated_segment = scene.GetNetSegmentById(segment_request.segment_id);
			assert(updated_segment);
			if (!updated_segment)
			{
				return false;
			}

			StoredSegmentUpdate segment_update{ updated_segment->get().GetId(), model::NetNodeId{0}, model::NetNodeId{0} , {}, {}};
			{
				// disconnect from old node
				auto node_id = updated_segment->get().m_firstNodeId;
				auto node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->get().GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && *segment == segment_request.segment_id)
					{
						segment_update.old_first_side = static_cast<model::ConnectedSegmentSide>(i);
						segment_update.old_node1 = node->get().GetId();
						node->get().SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			{
				// disconnect from old node
				auto node_id = updated_segment->get().m_secondNodeId;
				auto node = scene.GetNetNodeById(node_id);
				for (int i = 0; i < 4; i++)
				{
					auto segment = node->get().GetSegmentAt(static_cast<model::ConnectedSegmentSide>(i));
					if (segment && (*segment) == segment_request.segment_id)
					{
						segment_update.old_second_side = static_cast<model::ConnectedSegmentSide>(i);
						segment_update.old_node2 = node->get().GetId();
						node->get().SetSegmentAt(static_cast<model::ConnectedSegmentSide>(i), std::nullopt);
						break;
					}
				}
			}
			


			if (segment_request.node1_type == NetModificationRequest::NodeIdType::existing_id)
			{
				updated_segment->get().m_firstNodeId = segment_request.node1;
			}
			else
			{
				assert(static_cast<size_t>(segment_request.node1.value) < new_nodes.size());
				if (static_cast<size_t>(segment_request.node1.value) < new_nodes.size())
				{
					updated_segment->get().m_firstNodeId = new_nodes[segment_request.node1.value];
				}
			}
			if (segment_request.node2_type == NetModificationRequest::NodeIdType::existing_id)
			{
				updated_segment->get().m_secondNodeId = segment_request.node2;
			}
			else
			{
				assert(static_cast<size_t>(segment_request.node2.value) < new_nodes.size());
				if (static_cast<size_t>(segment_request.node2.value) < new_nodes.size())
				{
					updated_segment->get().m_secondNodeId = new_nodes[segment_request.node2.value];
				}
			}

			{
				auto node_id1 = updated_segment->get().m_firstNodeId;
				auto node1 = scene.GetNetNodeById(node_id1);
				assert(node1);
				auto node_id2 = updated_segment->get().m_secondNodeId;
				auto node2 = scene.GetNetNodeById(node_id2);
				assert(node2);

				if (!node1 || !node2)
				{
					return false;
				}

				node1->get().SetSegmentAt(segment_request.node1_side, updated_segment->get().GetId());
				node2->get().SetSegmentAt(segment_request.node2_side, updated_segment->get().GetId());
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

			if (!node1_id || !node2_id)
			{
				return false;
			}

			auto node1 = scene.GetNetNodeById(*node1_id);
			assert(node1);
			auto node2 = scene.GetNetNodeById(*node2_id);
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
			node1->get().SetSegmentAt(segment_request.node1_side, id);
			node2->get().SetSegmentAt(segment_request.node2_side, id);
		}

		// handle added connections
		for (auto&& connection : update_request.added_connections)
		{
			auto block = scene.GetBlockById(connection.socket.block_id);
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
				scene.AddSocketNodeConnection(model::SocketNodeConnection{ connection.socket , node_id });
				edits.stored_new_connections.push_back({ connection.socket, node_id });
			}
		}


		// setup notification
		NetModificationReport report;
		report.removed_connections = update_request.removed_connections;
		report.removed_nodes = update_request.removed_nodes;
		report.removed_segments = update_request.removed_segments;

		report.added_nodes.reserve(update_request.added_nodes.size());
		for (auto&& node_id : new_nodes)
		{
			auto node = scene.GetNetNodeById(node_id);
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
			auto segment = scene.GetNetSegmentById(update_segment_request.segment_id);
			assert(segment);
			report.update_segments.push_back(*segment);
		}

		report.added_segments.reserve(new_segments.size());
		for (auto&& added_segment : new_segments)
		{
			auto segment = scene.GetNetSegmentById(added_segment);
			assert(segment);
			report.added_segments.push_back(*segment);
		}

		report.added_connections.reserve(update_request.added_connections.size());
		for (auto&& added_connection : update_request.added_connections)
		{
			auto node_id = added_connection.node;
			if (added_connection.node_type == NetModificationRequest::NodeIdType::new_id)
			{
				assert(static_cast<size_t>(added_connection.node.value) < new_nodes.size());
				node_id = new_nodes[static_cast<size_t>(added_connection.node.value)];
			}
			auto conn = scene.GetSocketConnectionForNode(node_id);
			assert(conn);
			report.added_connections.push_back(*conn);
		}

		manager.Notify(SceneModification{ SceneModification::type_t::NetUpdated, SceneModification::data_t{std::ref(report)} });

		return true;
	}
};

}

node::SceneModelManager::SceneModelManager(std::shared_ptr<model::NodeSceneModel> scene)
	:m_scene(std::move(scene))
{
	assert(m_scene);
	for (const auto& block : m_scene->GetBlocks())
	{
		if (block.GetClass() == "SubSystem")
		{
			RegisterSubSystem(block);
		}
	}
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
	auto action = std::make_unique<AddBlockAction>(std::move(block));
	PushAction(std::move(action));
}

void node::SceneModelManager::RemoveBlockById(const model::BlockId& id)
{
	auto action = std::make_unique<RemoveBlockAction>(id);
	PushAction(std::move(action));
}

void node::SceneModelManager::ResizeBlockById(const model::BlockId& id, const model::Rect& new_rect, model::BlockOrientation new_orientation, std::vector<model::BlockSocketModel> socket_positions)
{
	auto action = std::make_unique<ResizeBlockAction>(new_rect, id, new_orientation, std::move(socket_positions));
	PushAction(std::move(action));
}

void node::SceneModelManager::MoveBlockById(const model::BlockId& id, const model::Point& new_origin)
{
	auto move_action = std::make_unique<MoveBlockAction>(new_origin, id);
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

void node::SceneModelManager::UpdateNet(NetModificationRequest& update_request)
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


void node::SceneModelManager::RegisterSubSystem(const model::BlockModel& model)
{
	auto& properties = model.GetProperties();
	auto it = std::find_if(properties.begin(), properties.end(), [&](const model::BlockProperty& p) { return p.name == "SubSceneId"; });
	if (it == properties.end())
	{
		assert(false);
		return;
	}
	auto value = static_cast<int32_t>(std::get<uint64_t>(it->prop));
	m_subsystem_ids[model.GetId()] = SubSceneId{ value };
}

void node::SceneModelManager::UnRegisterSubSystem(const model::BlockId& id)
{
	auto it = m_subsystem_ids.find(id);
	if (it != m_subsystem_ids.end())
	{
		m_subsystem_ids.erase(it);
	}
}
