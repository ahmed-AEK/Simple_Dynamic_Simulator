#include "NetDeleteLogic.hpp"
#include "NetObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"
#include "BlockSocketObject.hpp"
#include "BlockObject.hpp"

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
		std::array<NetNode*, 2> nodes_to_check{segment->getStartNode(), segment->getEndNode()};
		for (auto* node: nodes_to_check)
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

void AddHealedNodesToRequest(node::NetModificationRequest& request, const std::set<NodeToHealDescriptor>& nodes_to_heal)
{
	using namespace node;
	using namespace node::model;

	for (const NodeToHealDescriptor& node_to_heal : nodes_to_heal)
	{
		request.removed_nodes.push_back(*node_to_heal.node->GetId());
		request.removed_segments.push_back(*node_to_heal.segment1->GetId());
		request.removed_segments.push_back(*node_to_heal.segment2->GetId());

		NetNode* node1 = nullptr;
		if (node_to_heal.segment1->getStartNode() != node_to_heal.node)
		{
			node1 = node_to_heal.segment1->getStartNode();
		}
		else
		{
			node1 = node_to_heal.segment1->getEndNode();
		}

		NetNode* node2 = nullptr;
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
			NetModificationRequest::NodeIdType::existing_id,
			NetModificationRequest::NodeIdType::existing_id,
			side1,
			side2,
			node_to_heal.segment1->GetOrientation(),
			*node1->GetId(),
			*node2->GetId()
			});
	}

}

MI::ClickEvent node::logic::NetDeleteLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_object.isAlive() || new_hover != m_object.GetObjectPtr())
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	NetModificationRequest request;
	
	std::set<NetSegment*> segments_to_remove;
	std::set<NetNode*> nodes_to_remove;
	for (auto&& object : GetScene()->GetCurrentSelection())
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
				request.removed_segments.push_back(*id_opt);
				segments_to_remove.insert(net_segment);
			}
		}
		if (object_ptr->GetObjectType() == ObjectType::netNode)
		{
			auto* netnode = static_cast<NetNode*>(object_ptr);
			auto id_opt = netnode->GetId();
			assert(id_opt);
			if (id_opt)
			{
				request.removed_nodes.push_back(*id_opt);
				nodes_to_remove.insert(netnode);
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

	auto nodes_to_heal = GetNodesToHeal(segments_to_remove, nodes_to_remove);
	AddHealedNodesToRequest(request, nodes_to_heal);
	
	GetObjectsManager()->GetSceneModel()->UpdateNet(std::move(request));
	return MI::ClickEvent::CAPTURE_END;
}



