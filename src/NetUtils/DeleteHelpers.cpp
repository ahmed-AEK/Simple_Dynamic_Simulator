#include "DeleteHelpers.hpp"
#include "NetUtils/Utils.hpp"

struct NodeToHealDescriptor
{
	node::model::NetNodeId node;
	node::model::NetSegmentId segment1;
	node::model::NetSegmentId segment2;
	bool operator==(const NodeToHealDescriptor&) const = default;
};

template <>
struct std::hash<NodeToHealDescriptor>
{
	std::size_t operator()(const NodeToHealDescriptor& k) const
	{
		size_t hash = 0;
		boost::hash_combine(hash, k.node.value);
		boost::hash_combine(hash, k.segment1.value);
		boost::hash_combine(hash, k.segment2.value);
		return hash;
	}
};

static std::unordered_set<NodeToHealDescriptor> GetNodesToHeal(
	const std::unordered_set<node::model::NetSegmentId>& segments_to_remove,
	const std::unordered_set<node::model::NetNodeId>& nodes_to_remove, const node::model::NodeSceneModel& scene)
{
	using namespace node;
	using namespace node::model;
	std::unordered_set<NodeToHealDescriptor> result;
	for (const auto& segment_id_to_remove : segments_to_remove)
	{
		auto* segment_to_remove = scene.GetNetSegmentById(segment_id_to_remove);
		assert(segment_to_remove);
		std::array<const NetNodeModel*, 2> nodes_to_check{ 
			scene.GetNetNodeById(segment_to_remove->m_firstNodeId), scene.GetNetNodeById(segment_to_remove->m_secondNodeId) };
		assert(nodes_to_check[0]);
		assert(nodes_to_check[1]);
		for (auto* node : nodes_to_check)
		{
			if (node->GetConnectedSegmentsCount() < 3)
			{
				continue;
			}
			if (nodes_to_remove.find(node->GetId()) != nodes_to_remove.end())
			{
				continue;
			}

			std::array<NetSegmentId, 2> segments_not_removed{ NetSegmentId{0}, NetSegmentId{0} };
			int segments_not_removed_count = 0;
			for (int i = 0; i < 4; i++)
			{
				auto segment_id = node->GetSegmentAt(static_cast<ConnectedSegmentSide>(i));
				if (segment_id && segments_to_remove.find(*segment_id) == segments_to_remove.end())
				{
					if (segments_not_removed_count < 2)
					{
						segments_not_removed[segments_not_removed_count] = *segment_id;
					}
					segments_not_removed_count++;
				}
			}

			if (segments_not_removed_count != 2)
			{
				continue;
			}
			auto* first_segment = scene.GetNetSegmentById(segments_not_removed[0]);
			auto* second_segment = scene.GetNetSegmentById(segments_not_removed[1]);
			assert(first_segment);
			assert(second_segment);
			if (first_segment->m_orientation == second_segment->m_orientation)
			{
				result.insert(NodeToHealDescriptor{ node->GetId(), segments_not_removed[0], segments_not_removed[1]});
			}
		}
	}
	return result;
}

static void AddHealedNodesToRequest(node::NetModificationRequest& request, 
	const std::unordered_set<NodeToHealDescriptor>& nodes_to_heal, const node::model::NodeSceneModel& scene)
{
	using namespace node;
	using namespace node::model;

	for (const NodeToHealDescriptor& node_to_heal : nodes_to_heal)
	{
		request.removed_nodes.push_back(node_to_heal.node);
		request.removed_segments.push_back(node_to_heal.segment1);
		request.removed_segments.push_back(node_to_heal.segment2);

		const NetNodeModel* node1 = nullptr;
		auto* segment1 = scene.GetNetSegmentById(node_to_heal.segment1);
		assert(segment1);
		if (segment1->m_firstNodeId != node_to_heal.node)
		{

			node1 = scene.GetNetNodeById(segment1->m_firstNodeId);
		}
		else
		{
			node1 = scene.GetNetNodeById(segment1->m_secondNodeId);
		}
		assert(node1);

		const NetNodeModel* node2 = nullptr;
		auto* segment2 = scene.GetNetSegmentById(node_to_heal.segment2);
		assert(segment2);
		if (segment2->m_firstNodeId != node_to_heal.node)
		{

			node2 = scene.GetNetNodeById(segment2->m_firstNodeId);
		}
		else
		{
			node2 = scene.GetNetNodeById(segment2->m_secondNodeId);
		}
		assert(node2);

		ConnectedSegmentSide side1{};
		ConnectedSegmentSide side2{};

		{
			model::NetNodeModel node1_model{ NetNodeId{1}, node1->GetPosition() };
			model::NetNodeModel node2_model{ NetNodeId{2}, node2->GetPosition() };
			model::NetSegmentModel segment_model{ NetSegmentId{2}, node1_model.GetId(), node2_model.GetId(), segment1->m_orientation };
			NetUtils::connectSegementAndNodes(segment_model, node1_model, node2_model);
			side1 = *node1_model.GetSegmentSide(segment_model.GetId());
			side2 = *node2_model.GetSegmentSide(segment_model.GetId());
		}

		request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
			NetModificationRequest::IdType::existing_id,
			NetModificationRequest::IdType::existing_id,
			side1,
			side2,
			segment1->m_orientation,
			node1->GetId(),
			node2->GetId()
			});
	}
}

static std::set<const node::model::NetNodeModel*> GetNodesToCheckForNet(
	const std::unordered_set<node::model::NetSegmentId>& segments_to_remove,
	const std::unordered_set<node::model::NetNodeId>& nodes_to_remove, const node::model::NodeSceneModel& scene)
{
	using namespace node;
	using namespace node::model;
	std::set<const node::model::NetNodeModel*> nodes_to_check;

	auto add_nodes_connected_to_segment = [&](const model::NetSegmentModel& segment)
		{
			auto start_node_id = segment.m_firstNodeId;
			auto end_node_id = segment.m_secondNodeId;

			if (nodes_to_remove.find(start_node_id) == nodes_to_remove.end())
			{
				auto* node = scene.GetNetNodeById(start_node_id);
				assert(node);
				nodes_to_check.insert(node);
			}
			if (nodes_to_remove.find(end_node_id) == nodes_to_remove.end())
			{
				auto* node = scene.GetNetNodeById(end_node_id);
				assert(node);
				nodes_to_check.insert(node);
			}
		};
	for (const auto& segment_id : segments_to_remove)
	{
		auto* segment = scene.GetNetSegmentById(segment_id);
		assert(segment);
		add_nodes_connected_to_segment(*segment);
	}
	for (const auto& node_id : nodes_to_remove)
	{
		auto* node = scene.GetNetNodeById(node_id);
		assert(node);
		for (const auto& segment_id : node->GetSegments())
		{
			if (!segment_id || segments_to_remove.find(*segment_id) != segments_to_remove.end())
			{
				continue;
			}

			auto* segment = scene.GetNetSegmentById(*segment_id);
			assert(segment);
			add_nodes_connected_to_segment(*segment);
		}
	}

	return nodes_to_check;

}

struct NetNodes
{
	std::vector<const node::model::NetNodeModel*> nodes;
};

struct RemainingNetsReport
{
	std::unordered_map<node::model::NetId, std::vector<NetNodes>> nets;
};

static RemainingNetsReport GetNetsAfterRemoval(
	const std::unordered_set<node::model::NetSegmentId>& segments_to_remove,
	const std::unordered_set<node::model::NetNodeId>& nodes_to_remove, const node::model::NodeSceneModel& scene)
{
	using namespace node;
	using namespace node::model;

	std::set<const NetNodeModel*> nodes_to_check = GetNodesToCheckForNet(segments_to_remove, nodes_to_remove, scene);

	std::unordered_set<NetNodeId> visited_nodes = nodes_to_remove;
	RemainingNetsReport report;
	for (const auto& node_id : nodes_to_remove)
	{
		auto* node = scene.GetNetNodeById(node_id);
		assert(node);
		auto net_id = node->GetNetId();
		report.nets.insert({ net_id, {} });
	}

	for (auto* node_being_checked : nodes_to_check)
	{
		// node already visited
		if (visited_nodes.find(node_being_checked->GetId()) != visited_nodes.end())
		{
			continue;
		}

		visited_nodes.insert(node_being_checked->GetId());

		std::stack<const NetNodeModel*> nodes_to_visit;
		nodes_to_visit.push(node_being_checked);
		auto net_id = node_being_checked->GetNetId();

		NetNodes net;
		auto node_handler = [&](const NetNodeModel* node)
			{
				for (const auto& segment_id : node->GetSegments())
				{
					if (!segment_id || segments_to_remove.find(*segment_id) != segments_to_remove.end())
					{
						continue;
					}
					auto* segment = scene.GetNetSegmentById(*segment_id);
					assert(segment);
					auto start_node_id = segment->m_firstNodeId;
					auto end_node_id = segment->m_secondNodeId;
					if (visited_nodes.find(start_node_id) == visited_nodes.end())
					{
						auto* inner_node = scene.GetNetNodeById(start_node_id);
						assert(inner_node);
						nodes_to_visit.push(inner_node);
						visited_nodes.insert(start_node_id);
					}
					if (visited_nodes.find(end_node_id) == visited_nodes.end())
					{
						auto* inner_node = scene.GetNetNodeById(end_node_id);
						assert(inner_node);
						nodes_to_visit.push(inner_node);
						visited_nodes.insert(end_node_id);
					}
				}
			};

		while (nodes_to_visit.size())
		{
			auto* node = nodes_to_visit.top();
			nodes_to_visit.pop();
			net.nodes.push_back(node);
			node_handler(node);
		}

		report.nets[net_id].push_back(std::move(net));

	}

	return report;
}


static void AddNetCategoryChanges(node::NetModificationRequest& request, const RemainingNetsReport& nets_report, const node::model::NodeSceneModel& scene)
{
	using namespace node;
	using namespace node::model;

	node_int added_net_id = 0;
	for (const auto& [id, nets] : nets_report.nets)
	{
		if (nets.size() == 0)
		{
			request.removed_nets.push_back(id);
		}
		for (size_t i = 0; i < nets.size(); i++)
		{
			NetCategory new_category;
			auto* net_ptr = scene.GetNet(id);
			assert(net_ptr);
			NetCategory old_category = net_ptr->GetCategory();
			size_t nodes_in_net = 0;
			for (const auto& node : nets[i].nodes)
			{
				auto node_id = node->GetId();
				auto it = std::find(request.removed_nodes.begin(), request.removed_nodes.end(), node_id);
				if (it != request.removed_nodes.end())
				{
					continue;
				}
				if (auto* connection = scene.GetSocketConnectionForNode(node->GetId()))
				{
					auto* block = scene.GetBlockById(connection->socketId.block_id);
					assert(block);
					auto* socket = block->GetSocketById(connection->socketId.socket_id);
					const auto& socket_category = socket->GetCategory();
					assert(NetCategory::Joinable(new_category, socket_category));
					if (!socket_category.IsEmpty())
					{
						new_category = socket_category;
					}
				}
				nodes_in_net++;
			}
			if (i == 0)
			{
				if (nodes_in_net == 0)
				{
					request.removed_nets.push_back(id);
				}
				if (new_category != old_category)
				{
					request.changed_net_categories.push_back(NetModificationRequest::NetCategoryChange{ .net_id = id, .new_category = new_category });
				}
			}
			else if (nodes_in_net != 0) // && i != 0
			{
				auto new_net_id = added_net_id;
				added_net_id++;
				request.added_nets.push_back(NetModificationRequest::AddNetRequest{ .category = new_category });
				for (const auto& node : nets[i].nodes)
				{
					auto node_id = node->GetId();
					auto it = std::find(request.removed_nodes.begin(), request.removed_nodes.end(), node_id);
					if (it != request.removed_nodes.end())
					{
						continue;
					}
					request.update_nodes_nets.push_back(
						NetModificationRequest::NodeNetChange{ .net_type = NetModificationRequest::IdType::new_id,
						.node = node_id, .net = NetId{new_net_id} });
				}
			}
		}
	}
}

struct NetComponentsSet
{
	std::unordered_set<node::model::NetSegmentId> segments;
	std::unordered_set<node::model::NetNodeId> nodes;
};


static NetComponentsSet GetNetComponents(
	std::span<const node::model::NetSegmentId> segments_to_remove,
	std::span<const node::model::NetNodeId> nodes_to_remove)
{
	using namespace node;
	using namespace node::model;
	NetComponentsSet components;

	for (const auto& node_id : nodes_to_remove)
	{
		components.nodes.insert(node_id);
	}
	for (const auto& segment_id : segments_to_remove)
	{
		components.segments.insert(segment_id);
	}
	return components;
}

std::optional<node::NetModificationRequest> node::NetUtils::GetDeletionRequestForNet(std::span<const model::NetSegmentId> segments_to_remove, 
	std::span<const model::NetNodeId> nodes_to_remove, const model::NodeSceneModel& scene)
{
	using namespace node::model;

	if (segments_to_remove.size() == 0 && nodes_to_remove.size() == 0)
	{
		return {};
	}

	NetModificationRequest request;

	auto&& [segments_to_remove_set, nodes_to_remove_set] = GetNetComponents(segments_to_remove, nodes_to_remove);

	for (const auto& node_id : nodes_to_remove_set)
	{
		request.removed_nodes.push_back(node_id);
		if (auto* connection = scene.GetSocketConnectionForNode(node_id))
		{
			request.removed_connections.push_back(connection->socketId);
		}
	}
	for (const auto& segment_id : segments_to_remove)
	{
		request.removed_segments.push_back(segment_id);
	}

	[[maybe_unused]] auto nets_after_removal = GetNetsAfterRemoval(segments_to_remove_set, nodes_to_remove_set, scene);

	auto nodes_to_heal = GetNodesToHeal(segments_to_remove_set, nodes_to_remove_set, scene);
	AddHealedNodesToRequest(request, nodes_to_heal, scene);

	AddNetCategoryChanges(request, nets_after_removal, scene);

	return request;
}

std::optional<node::BlockDeletionRequest> node::NetUtils::GetDeletionRequestForBlock(model::BlockId block_id, const model::NodeSceneModel& scene)
{
	using namespace node::model;

	node::BlockDeletionRequest request{ .block_id = block_id };

	std::unordered_set<NetSegmentId> segments_to_remove;
	std::unordered_set<NetNodeId> nodes_to_remove;

	auto* block = scene.GetBlockById(block_id);
	assert(block);
	for (auto& socket : block->GetSockets())
	{
		auto unique_id = SocketUniqueId{ .socket_id = socket.GetId(), .block_id = block_id };
		auto* connection = scene.GetSocketConnectionForSocket(unique_id);
		if (connection)
		{
			nodes_to_remove.insert(connection->NodeId);
			request.net_modification.removed_connections.push_back(unique_id);
		}
	}

	if (request.net_modification.removed_connections.size())
	{
		[[maybe_unused]] auto nets_after_removal = GetNetsAfterRemoval(segments_to_remove, nodes_to_remove, scene);

		AddNetCategoryChanges(request.net_modification, nets_after_removal, scene);
	}

	return request;
}

std::optional<node::model::NetCategory> node::NetUtils::GetCategoryAfterRemoval(model::NetId net_id, std::span<const model::NetSegmentId> segments_to_remove,
	std::span<const model::NetNodeId> nodes_to_remove, const model::NodeSceneModel& scene)
{
	if (segments_to_remove.size() == 0 && nodes_to_remove.size() == 0)
	{
		return scene.GetNet(net_id)->GetCategory();
	}
	auto&& [segments_to_remove_set, nodes_to_remove_set] = GetNetComponents(segments_to_remove, nodes_to_remove);

	auto* net = scene.GetNet(net_id);
	assert(net);
	if (net->GetNodes().size() == 0)
	{
		return {};
	}
	std::optional<model::NetCategory> result;
	for (const auto& node_id : net->GetNodes())
	{
		if (nodes_to_remove_set.find(node_id) != nodes_to_remove_set.end())
		{
			continue;
		}
		auto* conn = scene.GetSocketConnectionForNode(node_id);
		if (!conn)
		{
			continue;
		}
		auto* block = scene.GetBlockById(conn->socketId.block_id);
		assert(block);
		if (!block)
		{
			continue;
		}
		auto* socket = block->GetSocketById(conn->socketId.socket_id);
		assert(socket);
		result = socket->GetCategory();
		if (!result->IsEmpty())
		{
			return result;
		}
	}
	return result;
}