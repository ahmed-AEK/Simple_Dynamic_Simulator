#include "NetDeleteLogic.hpp"
#include "NetObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"
#include "NetUtils/Utils.hpp"

#include "GraphicsScene/BlockSocketObject.hpp"
#include "NodeModels/SceneModelManager.hpp"

#include <set>

node::logic::NetDeleteLogic::NetDeleteLogic(GraphicsObject& object, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_object{ object.GetMIHandlePtr() }
{
}

void node::logic::NetDeleteLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	const auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_object.isAlive() || new_hover != m_object.GetObjectPtr())
	{
		GetScene()->SetGraphicsLogic(nullptr);
	}
}

MI::ClickEvent node::logic::NetDeleteLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	const auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_object.isAlive() || new_hover != m_object.GetObjectPtr())
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	auto scene = GetObjectsManager()->GetSceneModel();
	auto request = NetUtils::GetDeletionRequestForNet(GetScene()->GetCurrentSelection(), scene->GetModel());
	if (!request)
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	GetObjectsManager()->GetSceneModel()->UpdateNet(std::move(*request));
	return MI::ClickEvent::CAPTURE_END;
}



struct NodeToHealDescriptor
{
	node::NetNode* node;
	node::NetSegment* segment1;
	node::NetSegment* segment2;
	auto operator<=>(const NodeToHealDescriptor&) const = default;
};

static std::set<NodeToHealDescriptor> GetNodesToHeal(std::set<node::NetSegment*>& segments_to_remove,
	std::set<node::NetNode*> nodes_to_remove)
{
	using namespace node;
	using namespace node::model;
	std::set<NodeToHealDescriptor> result;
	for (auto* segment : segments_to_remove)
	{
		std::array<NetNode*, 2> nodes_to_check{ segment->getStartNode(), segment->getEndNode() };
		for (auto* node : nodes_to_check)
		{
			if (node->GetConnectedSegmentsCount() < 3 || !node->GetId().has_value())
			{
				continue;
			}
			if (nodes_to_remove.find(node) != nodes_to_remove.end())
			{
				continue;
			}

			std::array<node::NetSegment*, 2> segments_not_removed{};
			int segments_not_removed_count = 0;
			for (int i = 0; i < 4; i++)
			{
				segment = node->getSegment(static_cast<ConnectedSegmentSide>(i));
				if (segment && segments_to_remove.find(segment) == segments_to_remove.end())
				{
					if (segments_not_removed_count < 2)
					{
						segments_not_removed[segments_not_removed_count] = segment;
					}
					segments_not_removed_count++;
				}
			}

			if (segments_not_removed_count != 2)
			{
				continue;
			}

			if (segments_not_removed[0]->GetOrientation() == segments_not_removed[1]->GetOrientation())
			{
				result.insert(NodeToHealDescriptor{ node, segments_not_removed[0], segments_not_removed[1] });
			}
		}
	}
	return result;
}

static void AddHealedNodesToRequest(node::NetModificationRequest& request, const std::set<NodeToHealDescriptor>& nodes_to_heal)
{
	using namespace node;
	using namespace node::model;

	for (const NodeToHealDescriptor& node_to_heal : nodes_to_heal)
	{
		request.removed_nodes.push_back(*node_to_heal.node->GetId());
		request.removed_segments.push_back(*node_to_heal.segment1->GetId());
		request.removed_segments.push_back(*node_to_heal.segment2->GetId());

		const NetNode* node1 = nullptr;
		if (node_to_heal.segment1->getStartNode() != node_to_heal.node)
		{
			node1 = node_to_heal.segment1->getStartNode();
		}
		else
		{
			node1 = node_to_heal.segment1->getEndNode();
		}

		const NetNode* node2 = nullptr;
		if (node_to_heal.segment2->getStartNode() != node_to_heal.node)
		{
			node2 = node_to_heal.segment2->getStartNode();
		}
		else
		{
			node2 = node_to_heal.segment2->getEndNode();
		}

		ConnectedSegmentSide side1{};
		ConnectedSegmentSide side2{};

		{
			model::NetNodeModel node1_model{ NetNodeId{1}, node1->getCenter() };
			model::NetNodeModel node2_model{ NetNodeId{2}, node2->getCenter() };
			model::NetSegmentModel segment_model{ NetSegmentId{2}, node1_model.GetId(), node2_model.GetId(), node_to_heal.segment1->GetOrientation() };
			NetUtils::connectSegementAndNodes(segment_model, node1_model, node2_model);
			side1 = *node1_model.GetSegmentSide(segment_model.GetId());
			side2 = *node2_model.GetSegmentSide(segment_model.GetId());
		}

		request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
			NetModificationRequest::IdType::existing_id,
			NetModificationRequest::IdType::existing_id,
			side1,
			side2,
			node_to_heal.segment1->GetOrientation(),
			*node1->GetId(),
			*node2->GetId()
			});
	}
}

std::set<node::NetNode*> GetNodesToCheckForNet(
	const std::set<node::NetSegment*>& segments_to_remove,
	const std::set<node::NetNode*>& nodes_to_remove)
{
	using namespace node;
	std::set<NetNode*> nodes_to_check;

	auto add_nodes_connected_to_segment = [&](NetSegment* segment)
		{
			auto* start_node = segment->getStartNode();
			auto* end_node = segment->getEndNode();
			if (nodes_to_remove.find(start_node) == nodes_to_remove.end())
			{
				nodes_to_check.insert(start_node);
			}
			if (nodes_to_remove.find(end_node) == nodes_to_remove.end())
			{
				nodes_to_check.insert(end_node);
			}
		};
	for (auto* segment : segments_to_remove)
	{
		add_nodes_connected_to_segment(segment);
	}
	for (auto* node : nodes_to_remove)
	{
		for (auto* segment : node->getSegments())
		{
			if (!segment || segments_to_remove.find(segment) != segments_to_remove.end())
			{
				continue;
			}
			add_nodes_connected_to_segment(segment);
		}
	}

	return nodes_to_check;

}

struct NetNodes
{
	std::vector<node::NetNode*> nodes;
};

struct RemainingNetsReport
{
	std::unordered_map<node::model::NetId, std::vector<NetNodes>> nets;
};

RemainingNetsReport GetNetsAfterRemoval(
	const std::set<node::NetSegment*>& segments_to_remove,
	const std::set<node::NetNode*>& nodes_to_remove)
{
	using namespace node;

	std::set<NetNode*> nodes_to_check = GetNodesToCheckForNet(segments_to_remove, nodes_to_remove);

	std::set<NetNode*> visited_nodes = nodes_to_remove;
	RemainingNetsReport report;
	for (const auto& node : nodes_to_remove)
	{
		auto net_id = node->GetNetId();
		assert(net_id);
		if (!net_id)
		{
			continue;
		}
		report.nets.insert({ *net_id, {} });
	}
	for (auto* node_being_checked : nodes_to_check)
	{
		// node already visited
		if (visited_nodes.find(node_being_checked) != visited_nodes.end())
		{
			continue;
		}

		visited_nodes.insert(node_being_checked);

		std::stack<NetNode*> nodes_to_visit;
		nodes_to_visit.push(node_being_checked);
		auto net_id = node_being_checked->GetNetId();
		assert(net_id);
		if (!net_id)
		{
			continue;
		}

		NetNodes net;
		auto node_handler = [&](NetNode* node)
			{
				for (auto* segment : node->getSegments())
				{
					if (!segment)
					{
						continue;
					}
					if (segments_to_remove.find(segment) != segments_to_remove.end())
					{
						continue;
					}
					auto* start_node = segment->getStartNode();
					auto* end_node = segment->getEndNode();
					if (visited_nodes.find(start_node) == visited_nodes.end())
					{
						nodes_to_visit.push(start_node);
						visited_nodes.insert(start_node);
					}
					if (visited_nodes.find(end_node) == visited_nodes.end())
					{
						nodes_to_visit.push(end_node);
						visited_nodes.insert(end_node);
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

		report.nets[*net_id].push_back(std::move(net));

	}

	return report;
}


void AddNetCategoryChanges(node::NetModificationRequest& request, const RemainingNetsReport& nets_report, const node::model::NodeSceneModel& scene)
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
				assert(node_id);
				if (!node_id)
				{
					continue;
				}
				auto it = std::find(request.removed_nodes.begin(), request.removed_nodes.end(), *node_id);
				if (it != request.removed_nodes.end())
				{
					continue;
				}
				if (auto* socket = node->GetConnectedSocket())
				{
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
					assert(node_id);
					if (!node_id)
					{
						continue;
					}
					auto it = std::find(request.removed_nodes.begin(), request.removed_nodes.end(), *node_id);
					if (it != request.removed_nodes.end())
					{
						continue;
					}
					request.update_nodes_nets.push_back(
						NetModificationRequest::NodeNetChange{ .net_type = NetModificationRequest::IdType::new_id,
						.node = *node_id, .net = NetId{new_net_id} });
				}
			}
		}
	}
}

std::optional<node::NetModificationRequest> node::NetUtils::GetDeletionRequestForNet(
	std::span<node::HandlePtr<node::GraphicsObject>> objects, const model::NodeSceneModel& scene)
{
	using namespace node;
	if (objects.size() == 0)
	{
		return {};
	}

	NetModificationRequest request;

	std::set<NetSegment*> segments_to_remove;
	std::set<NetNode*> nodes_to_remove;
	for (auto&& object : objects)
	{
		auto* object_ptr = object.GetObjectPtr();
		if (!object_ptr)
		{
			continue;
		}
		if (object_ptr->GetObjectType() == ObjectType::netSegment)
		{
			auto* net_segment = static_cast<NetSegment*>(object_ptr);
			auto id_opt = net_segment->GetId();
			assert(id_opt);
			if (id_opt)
			{
				auto&& [it, inserted] = segments_to_remove.insert(net_segment);
				if (inserted)
				{
					request.removed_segments.push_back(*id_opt);
				}
			}
		}
		if (object_ptr->GetObjectType() == ObjectType::netNode)
		{
			auto* netnode = static_cast<NetNode*>(object_ptr);
			auto id_opt = netnode->GetId();
			assert(id_opt);
			if (id_opt)
			{
				auto&& [it, inserted] = nodes_to_remove.insert(netnode);
				if (inserted)
				{
					request.removed_nodes.push_back(*id_opt);
				}
			}
			if (netnode->GetConnectedSocket())
			{
				auto block_id_opt = netnode->GetConnectedSocket()->GetUniqueId();
				assert(block_id_opt);
				if (block_id_opt)
				{
					request.removed_connections.push_back(*block_id_opt);
				}
			}
		}
	}
	if (segments_to_remove.size() == 0 && nodes_to_remove.size() == 0)
	{
		return {};
	}
	[[maybe_unused]] auto nets_after_removal = GetNetsAfterRemoval(segments_to_remove, nodes_to_remove);

	auto nodes_to_heal = GetNodesToHeal(segments_to_remove, nodes_to_remove);
	AddHealedNodesToRequest(request, nodes_to_heal);

	AddNetCategoryChanges(request, nets_after_removal, scene);

	return request;
}
