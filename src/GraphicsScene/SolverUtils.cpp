#include "GraphicsScene/SolverUtils.hpp"

#include "GraphicsObjectsManager.hpp"
#include "BlockSocketObject.hpp"

node::logic::NetBranch node::logic::GetNetBranchForLeafNode(node::NetNode& leaf_node)
{
	NetBranch branch;
	auto* current_node = &leaf_node;

	NetSegment* connected_segment = nullptr;

	branch.nodes.emplace_back(*current_node);

	while (current_node->GetConnectedSegmentsCount() < 3)
	{
		auto* other_segment = [&]() -> NetSegment*
			{
				for (int i = 0; i < 4; i++)
				{
					if (auto* segment = current_node->getSegment(static_cast<model::ConnectedSegmentSide>(i)); segment && segment != connected_segment)
					{
						return segment;
					}
				}
				return nullptr;
			}();
		if (other_segment == nullptr)
		{
			break;
		}
		branch.segments.emplace_back(*other_segment);
		connected_segment = other_segment;
		auto* other_node = [&]() -> NetNode*
			{
				if (connected_segment->getStartNode() != current_node)
				{
					return connected_segment->getStartNode();
				}
				else
				{
					return connected_segment->getEndNode();
				}
			}();
		assert(other_node);
		current_node = other_node;
		branch.nodes.emplace_back(*current_node);
	}

	return branch;
}

void node::logic::ApplySolutionToNodes(const node::NetsSolver::NetSolution& solution, std::span<node::HandlePtrS<node::NetNode, node::GraphicsObject>> nodes,
	std::span<node::HandlePtrS<node::NetSegment, node::GraphicsObject>> segments)
{
	using namespace node;
	using namespace model;
	assert(solution.nodes.size() <= nodes.size());
	assert(solution.segments.size() <= segments.size());
	for (size_t i = 0; i < solution.nodes.size(); i++)
	{
		nodes[i]->setCenter(solution.nodes[i].GetPosition());
		nodes[i]->SetVisible(true);
	}
	for (size_t i = 0; i < solution.segments.size(); i++)
	{
		segments[i]->Connect(nodes[solution.nodes[i].GetId().value].GetObjectPtr(), nodes[solution.nodes[i + 1].GetId().value].GetObjectPtr(), solution.segments[i].m_orientation);
		segments[i]->SetVisible(true);
	}
}

node::logic::ModificationReport node::logic::MakeModificationsReport(const node::NetsSolver::NetSolution& solution,
	std::span<const node::HandlePtrS<node::NetNode, node::GraphicsObject>> orig_nodes, 
	std::span<const node::HandlePtrS<node::NetSegment, node::GraphicsObject>> orig_segments)
{
	ModificationReport report;
	auto& request = report.request;
	if (orig_nodes.size() > solution.nodes.size())
	{
		for (size_t i = solution.nodes.size(); i < orig_nodes.size(); i++)
		{
			request.removed_nodes.push_back(*orig_nodes[i]->GetId());
		}
	}
	if (orig_segments.size() > solution.segments.size())
	{
		for (size_t i = solution.segments.size(); i < orig_segments.size(); i++)
		{
			request.removed_segments.push_back(*orig_segments[i]->GetId());
		}
	}

	std::vector<NetNodeModificationInfo> new_nodes;
	{
		size_t current_node = 0;
		while (current_node < orig_nodes.size() && current_node < solution.nodes.size())
		{
			request.update_nodes.push_back(NetModificationRequest::UpdateNodeRequest{
				*orig_nodes[current_node]->GetId(),
				solution.nodes[current_node].GetPosition()
				});
			new_nodes.push_back(NetNodeModificationInfo{ NetModificationRequest::NodeIdType::existing_id,
				*orig_nodes[current_node]->GetId(),
				static_cast<size_t>(current_node)
				});
			current_node++;
		}
		while (current_node < solution.nodes.size())
		{
			request.added_nodes.push_back(NetModificationRequest::AddNodeRequest{
				solution.nodes[current_node].GetPosition()
				});
			

			new_nodes.push_back(NetNodeModificationInfo{ 
			NetModificationRequest::NodeIdType::new_id,
			model::NetNodeId{ static_cast<model::id_int>(request.added_nodes.size() - 1) },
			request.added_nodes.size() - 1
			});
			current_node++;
		}
	}
	report.start_node_info = new_nodes[0];
	report.end_node_info = new_nodes.back();
	{
		size_t current_segment = 0;
		while (current_segment < orig_segments.size() && current_segment < solution.segments.size())
		{
			auto& segment1_info = new_nodes[current_segment];
			auto& segment2_info = new_nodes[current_segment + 1];
			model::ConnectedSegmentSide side1 = 
				*solution.nodes[current_segment].GetSegmentSide(solution.segments[current_segment].GetId());
			model::ConnectedSegmentSide side2 = 
				*solution.nodes[current_segment + 1].GetSegmentSide(solution.segments[current_segment].GetId());
			request.update_segments.push_back(NetModificationRequest::UpdateSegmentRequest{
				segment1_info.node_type,
				segment2_info.node_type,
				side1,
				side2,
				solution.segments[current_segment].m_orientation,
				*orig_segments[current_segment]->GetId(),
				segment1_info.node_id,
				segment2_info.node_id
				});
			current_segment++;
		}
		while (current_segment < solution.segments.size())
		{
			auto& segment1_info = new_nodes[current_segment];
			auto& segment2_info = new_nodes[current_segment + 1];
			model::ConnectedSegmentSide side1 =
				*solution.nodes[current_segment].GetSegmentSide(solution.segments[current_segment].GetId());
			model::ConnectedSegmentSide side2 =
				*solution.nodes[current_segment + 1].GetSegmentSide(solution.segments[current_segment].GetId());
			request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
				segment1_info.node_type,
				segment2_info.node_type,
				side1,
				side2,
				solution.segments[current_segment].m_orientation,
				segment1_info.node_id,
				segment2_info.node_id
				});
			current_segment++;
		}
	}
	return report;
}

node::NetModificationRequest node::logic::MakeCreationReport(const node::NetsSolver::NetSolution& solution)
{
	NetModificationRequest request;
	
	for (const auto& node : solution.nodes)
	{
		request.added_nodes.push_back(NetModificationRequest::AddNodeRequest{
			node.GetPosition()
			});
	}

	for (const auto& segment : solution.segments)
	{
		auto segment1_side = model::ConnectedSegmentSide{};
		auto segment2_side = model::ConnectedSegmentSide{};
		if (segment.m_orientation == model::NetSegmentOrientation::horizontal)
		{
			if (solution.nodes[segment.m_firstNodeId.value].GetPosition().x <= solution.nodes[segment.m_secondNodeId.value].GetPosition().x) // node 0 on left
			{
				segment1_side = model::ConnectedSegmentSide::east;
				segment2_side = model::ConnectedSegmentSide::west;
			}
			else
			{
				segment1_side = model::ConnectedSegmentSide::west;
				segment2_side = model::ConnectedSegmentSide::east;
			}
		}
		if (segment.m_orientation == model::NetSegmentOrientation::vertical)
		{
			if (solution.nodes[segment.m_firstNodeId.value].GetPosition().y <= solution.nodes[segment.m_secondNodeId.value].GetPosition().y) // node 0 on top
			{
				segment1_side = model::ConnectedSegmentSide::south;
				segment2_side = model::ConnectedSegmentSide::north;
			}
			else
			{
				segment1_side = model::ConnectedSegmentSide::north;
				segment2_side = model::ConnectedSegmentSide::south;
			}
		}
		request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
			NetModificationRequest::NodeIdType::new_id,
			NetModificationRequest::NodeIdType::new_id,
			segment1_side,
			segment2_side,
			segment.m_orientation,
			segment.m_firstNodeId,
			segment.m_secondNodeId
			});
	}
	return request;
}

void node::logic::UpdateModificationEndWithSocket(
	std::span<const node::HandlePtrS<node::NetNode, node::GraphicsObject>> orig_nodes, 
	ModificationReport& report, const node::BlockSocketObject* end_socket)
{
	auto& request = report.request;
	bool end_disconnected = false;
	if (!end_socket)
	{
		// no end socket, just desconnect the last node
		auto* old_end_socket = orig_nodes.back()->GetConnectedSocket();
		if (orig_nodes.size() && old_end_socket)
		{
			request.removed_connections.push_back(*old_end_socket->GetUniqueId());
			end_disconnected = true;
		}
		return;
	}

	// there is an end_socket
	// disconnect it if it is not the last socket
	if (orig_nodes.size())
	{
		auto* connected_socket = orig_nodes.back()->GetConnectedSocket();
		if (connected_socket != end_socket) // need to create uptes
		{
			if (connected_socket)
			{
				request.removed_connections.push_back(*connected_socket->GetUniqueId());
				end_disconnected = true;
			}
			if (report.end_node_info.node_id == orig_nodes.back()->GetId())
			{
				// end was disconnected, a new socket needs to be connected
				request.added_connections.push_back(NetModificationRequest::SocketConnectionRequest{
					*end_socket->GetUniqueId(),
					report.end_node_info.node_type,
					report.end_node_info.node_id
					});
			}
		}
	}

	if (report.end_node_info.node_type == NetModificationRequest::NodeIdType::new_id ||
		(orig_nodes.size() && report.end_node_info.node_id != orig_nodes.back()->GetId())
		)
	{
		if (!end_disconnected)
		{
			// diconnect end_socket if it is the same socket but not the same end node
			auto* connected_socket = orig_nodes.back()->GetConnectedSocket();
			if (connected_socket)
			{
				request.removed_connections.push_back(*connected_socket->GetUniqueId());
				end_disconnected = true;
			}
		}

		// connect the new socket
		request.added_connections.push_back(NetModificationRequest::SocketConnectionRequest{
			*end_socket->GetUniqueId(),
			report.end_node_info.node_type,
			report.end_node_info.node_id
			});
	}
	
}

void node::logic::UpdateCreationStartWithSegment(
	NetModificationRequest& request, const node::NetSegment& start_segment)
{
	// connect to a new node
	const auto* node1 = start_segment.getStartNode();
	const auto* node2 = start_segment.getEndNode();
	auto node1_side = *node1->GetSegmentSide(start_segment);
	auto node2_side = *node2->GetSegmentSide(start_segment);
	// make the segment between node1 and our new end node
	request.update_segments.push_back(NetModificationRequest::UpdateSegmentRequest{
		NetModificationRequest::NodeIdType::existing_id,
		NetModificationRequest::NodeIdType::new_id,
		node1_side,
		model::GetOppositeSegmentSide(node1_side),
		start_segment.GetOrientation(),
		*start_segment.GetId(),
		*node1->GetId(),
		model::NetNodeId{0}
		});
	request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
		NetModificationRequest::NodeIdType::new_id,
		NetModificationRequest::NodeIdType::existing_id,
		model::GetOppositeSegmentSide(node2_side),
		node2_side,
		start_segment.GetOrientation(),
		model::NetNodeId{0},
		*node2->GetId()
		});
}

void node::logic::UpdateModificationEndWithSegment(std::span<const node::HandlePtrS<node::NetNode, node::GraphicsObject>> orig_nodes, 
	NetModificationRequest& request, const NetNodeModificationInfo& end_info, const node::NetSegment* end_segment)
{
	if (orig_nodes.size())
	{
		// no end socket, just desconnect the last node
		auto* old_end_socket = orig_nodes.back()->GetConnectedSocket();
		if (old_end_socket)
		{
			request.removed_connections.push_back(*old_end_socket->GetUniqueId());
		}
	}

	if (orig_nodes.size() && end_info.node_id == orig_nodes.back()->GetId())
	{
		// connect to existing node
		const auto* orig_node = orig_nodes.back().GetObjectPtr();
		const auto* node1 = end_segment->getStartNode();
		const auto* node2 = end_segment->getEndNode();
		auto node1_side = *node1->GetSegmentSide(*end_segment);
		auto node2_side = *node2->GetSegmentSide(*end_segment);
		// make the segment between node1 and our new end node
		request.update_segments.push_back(NetModificationRequest::UpdateSegmentRequest{
			NetModificationRequest::NodeIdType::existing_id,
			NetModificationRequest::NodeIdType::existing_id,
			node1_side,
			model::GetOppositeSegmentSide(node1_side),
			end_segment->GetOrientation(),
			*end_segment->GetId(),
			*node1->GetId(),
			*orig_node->GetId()
			});
		request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
			NetModificationRequest::NodeIdType::existing_id,
			NetModificationRequest::NodeIdType::existing_id,
			model::GetOppositeSegmentSide(node2_side),
			node2_side,
			end_segment->GetOrientation(),
			*orig_node->GetId(),
			*node2->GetId()
			});
	}
	else
	{
		// connect to a new node
		const auto* node1 = end_segment->getStartNode();
		const auto* node2 = end_segment->getEndNode();
		auto node1_side = *node1->GetSegmentSide(*end_segment);
		auto node2_side = *node2->GetSegmentSide(*end_segment);
		// make the segment between node1 and our new end node
		request.update_segments.push_back(NetModificationRequest::UpdateSegmentRequest{
			NetModificationRequest::NodeIdType::existing_id,
			NetModificationRequest::NodeIdType::new_id,
			node1_side,
			model::GetOppositeSegmentSide(node1_side),
			end_segment->GetOrientation(),
			*end_segment->GetId(),
			*node1->GetId(),
			model::NetNodeId{static_cast<model::id_int>(request.added_nodes.size() - 1)}
			});
		request.added_segments.push_back(NetModificationRequest::AddSegmentRequest{
			NetModificationRequest::NodeIdType::new_id,
			NetModificationRequest::NodeIdType::existing_id,
			model::GetOppositeSegmentSide(node2_side),
			node2_side,
			end_segment->GetOrientation(),
			model::NetNodeId{static_cast<model::id_int>(request.added_nodes.size() - 1)},
			*node2->GetId()
			});
	}
}


void node::logic::MergeModificationRequests(const node::NetModificationRequest& src,
	node::NetModificationRequest& target)
{
	using namespace node;
	using namespace node::model;

	std::unordered_map<NetNodeId, NetNodeId> node_id_map;

	target.removed_connections.insert(target.removed_connections.end(),
		src.removed_connections.begin(), src.removed_connections.end());
	target.removed_nodes.insert(target.removed_nodes.end(),
		src.removed_nodes.begin(), src.removed_nodes.end());
	target.removed_segments.insert(target.removed_segments.end(),
		src.removed_segments.begin(), src.removed_segments.end());
	target.update_nodes.insert(target.update_nodes.end(),
		src.update_nodes.begin(), src.update_nodes.end());
	for (size_t i = 0; i < src.added_nodes.size(); i++)
	{
		node_id_map.emplace(NetNodeId{ static_cast<id_int>(i) },
			NetNodeId{ static_cast<id_int>(target.added_nodes.size()) });
		target.added_nodes.push_back(src.added_nodes[i]);
	}
	for (size_t i = 0; i < src.added_segments.size(); i++)
	{
		target.added_segments.push_back(src.added_segments[i]);
		auto& segment = target.added_segments.back();
		if (segment.node1_type == NetModificationRequest::NodeIdType::new_id)
		{
			auto it = node_id_map.find(segment.node1);
			assert(it != node_id_map.end());
			segment.node1 = it->second;
		}
		if (segment.node2_type == NetModificationRequest::NodeIdType::new_id)
		{
			auto it = node_id_map.find(segment.node2);
			assert(it != node_id_map.end());
			segment.node2 = it->second;
		}
	}
	for (size_t i = 0; i < src.update_segments.size(); i++)
	{
		target.update_segments.push_back(src.update_segments[i]);
		auto& segment = target.update_segments.back();
		if (segment.node1_type == NetModificationRequest::NodeIdType::new_id)
		{
			auto it = node_id_map.find(segment.node1);
			assert(it != node_id_map.end());
			segment.node1 = it->second;
		}
		if (segment.node2_type == NetModificationRequest::NodeIdType::new_id)
		{
			auto it = node_id_map.find(segment.node2);
			assert(it != node_id_map.end());
			segment.node2 = it->second;
		}
	}
	for (auto&& src_conn : src.added_connections)
	{
		target.added_connections.push_back(src_conn);
		auto& conn = target.added_connections.back();
		if (conn.node_type == NetModificationRequest::NodeIdType::new_id)
		{
			auto it = node_id_map.find(conn.node);
			assert(it != node_id_map.end());
			conn.node = it->second;
		}
	}
}