#include "LeafNetNodeDragLogic.hpp"
#include "NetObject.hpp"
#include "GraphicsObjectsManager.hpp"
#include "BlockObject.hpp"
#include "BlockSocketObject.hpp"
#include "NetUtils/NetsSolver.hpp"
#include "GraphicsScene/SolverUtils.hpp"



std::unique_ptr<node::logic::LeafNetNodeDragLogic> node::logic::LeafNetNodeDragLogic::TryCreate(NetNode& leaf_node,
	GraphicsScene& scene, GraphicsObjectsManager& manager)
{
	assert(leaf_node.GetConnectedSegmentsCount() == 1);
	TemporaryNetManager net_mgr = TemporaryNetManager::CreateFromLeafNodeNet(leaf_node, scene);

	return std::make_unique<LeafNetNodeDragLogic>(std::move(net_mgr), &scene, &manager);
}

node::logic::LeafNetNodeDragLogic::LeafNetNodeDragLogic(TemporaryNetManager net_manager, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{ scene, manager } , m_temp_net_mgr{std::move(net_manager)}
{
	PositionNodes(m_temp_net_mgr.orig_nodes.back()->getCenter());
}

void node::logic::LeafNetNodeDragLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	PositionNodes(current_mouse_point);
}

MI::ClickEvent node::logic::LeafNetNodeDragLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	CleanUp();
	if (!std::visit(AnchorAlive{}, m_temp_net_mgr.start_anchor))
	{
		return MI::ClickEvent::CAPTURE_END;
	}
	const auto* end_socket = GetSocketAt(current_mouse_point);
	if (end_socket && end_socket == m_temp_net_mgr.orig_nodes.back()->GetConnectedSocket())
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	assert(GetObjectsManager());

	auto request = PopulateResultNet(current_mouse_point);
	GetObjectsManager()->GetSceneModel()->UpdateNet(std::move(request));

	return MI::ClickEvent::CAPTURE_END;
}



void node::logic::LeafNetNodeDragLogic::OnCancel()
{
	CleanUp();
}

node::BlockSocketObject* node::logic::LeafNetNodeDragLogic::GetSocketAt(const model::Point& point) const
{
	const auto& blocks = GetObjectsManager()->getBlocksRegistry();

	auto current_mouse_point_SDL = ToSDLPoint(point);
	BlockSocketObject* end_socket = nullptr;
	for (const auto& block_it : blocks)
	{
		const auto& sockets = block_it.second->GetSockets();
		auto it = std::find_if(sockets.begin(), sockets.end(), [&](const auto& socket)
			{
				auto socket_rect = ToSDLRect(socket->GetSceneRect());
				if (SDL_PointInRectFloat(&current_mouse_point_SDL, &socket_rect))
				{
					return true;
				}
				return false;
			});
		if (it != sockets.end())
		{
			auto* connected_node = it->get()->GetConnectedNode();
			if (connected_node == nullptr || connected_node == m_temp_net_mgr.orig_nodes.back().GetObjectPtr())
			{
				end_socket = it->get();
				break;
			}
		}
	}
	return end_socket;
}

node::NetSegment* node::logic::LeafNetNodeDragLogic::GetSegmentAt(const model::Point& point) const
{
	const auto& segments = GetObjectsManager()->getSegmentsRegistry();

	auto current_mouse_point_SDL = ToSDLPoint(point);
	NetSegment* end_segment = nullptr;
	for (const auto& segment_it : segments)
	{
		if (!segment_it.second->IsVisible())
		{
			continue;
		}
		auto segment_rect = ToSDLRect(segment_it.second->GetSceneRect());
		if (!SDL_PointInRectFloat(&current_mouse_point_SDL, &segment_rect))
		{
			continue;
		}
		auto it = std::find_if(m_temp_net_mgr.orig_segments.begin(),
			m_temp_net_mgr.orig_segments.end(),
			[&](const auto& orig_segment_it) { return orig_segment_it.GetObjectPtr() == segment_it.second; });
		if (it != m_temp_net_mgr.orig_segments.end())
		{
			continue;
		}

		end_segment = segment_it.second;
	}
	return end_segment;
}

static node::model::NetCategory GetNetCategoryExcludingSocket(const node::model::NodeSceneModel& model, node::model::NetId net_id, node::model::SocketUniqueId excluded_id)
{
	using namespace node;
	using namespace node::model;

	auto* net = model.GetNet(net_id);
	assert(net);
	for (const auto& node_id : net->GetNodes())
	{
		auto* connection = model.GetSocketConnectionForNode(node_id);
		if (!connection || connection->socketId == excluded_id)
		{
			continue;
		}
		auto* block = model.GetBlockById(connection->socketId.block_id);
		assert(block);
		auto* socket = block->GetSocketById(connection->socketId.socket_id);
		assert(socket);
		if (!socket->GetCategory().IsEmpty())
		{
			return socket->GetCategory();
		}
	}
	return NetCategory{};
}

void node::logic::LeafNetNodeDragLogic::AddNetModificationsWithSegment(NetModificationRequest& request, const NetSegment& end_segment) const
{
	const auto& scene_model = GetObjectsManager()->GetSceneModel()->GetModel();
	auto* old_end_socket = m_temp_net_mgr.orig_nodes.back()->GetConnectedSocket();
	if (old_end_socket && !old_end_socket->GetCategory().IsEmpty())
	{
		// we disconnected a non empty socket category, we should change net category if needed
		const auto& old_socket_category = old_end_socket->GetCategory();
		auto old_net_id_opt = m_temp_net_mgr.orig_nodes.back()->GetNetId();
		assert(old_net_id_opt);
		auto new_segment_net_id_opt = end_segment.getEndNode()->GetNetId();
		assert(new_segment_net_id_opt);

		model::NetCategory new_category_after_socket_removal = GetNetCategoryExcludingSocket(scene_model, *old_net_id_opt, *old_end_socket->GetUniqueId());

		if (new_category_after_socket_removal != old_socket_category)
		{
			request.changed_net_categories.push_back(
				NetModificationRequest::NetCategoryChange{ .net_id = *old_net_id_opt, .new_category = new_category_after_socket_removal });
		}

		if (old_net_id_opt != new_segment_net_id_opt)
		{
			// we connected to other net and disconnect our socket
			request.merged_nets.push_back(
				NetModificationRequest::MergeNetsRequest{ .original_net = *old_net_id_opt , .merged_net = *new_segment_net_id_opt });
		}
	}
	else
	{
		// disconnection has no effet on the net category, we just need to do the merge
		auto old_net_id_opt = m_temp_net_mgr.orig_nodes.back()->GetNetId();
		assert(old_net_id_opt);
		auto new_segment_net_id_opt = end_segment.getEndNode()->GetNetId();
		assert(new_segment_net_id_opt);

		if (old_net_id_opt == new_segment_net_id_opt)
		{
			return;
		}

		// we are connected to the other net, we need to merge

		assert(model::NetCategory::Joinable(
			scene_model.GetNet(*old_net_id_opt)->GetCategory(), scene_model.GetNet(*new_segment_net_id_opt)->GetCategory()));
		request.merged_nets.push_back(NetModificationRequest::MergeNetsRequest{ .original_net = *old_net_id_opt, .merged_net = *new_segment_net_id_opt });
	}
}

void node::logic::LeafNetNodeDragLogic::AddNetModificationsWithSocket(NetModificationRequest& request, const BlockSocketObject* end_socket) const
{
	const auto& scene_model = GetObjectsManager()->GetSceneModel()->GetModel();
	auto* old_end_socket = m_temp_net_mgr.orig_nodes.back()->GetConnectedSocket();
	if (old_end_socket)
	{
		// we disconnected a socket, we should change net category if needed
		const auto& old_socket_category = old_end_socket->GetCategory();
		auto old_net_id_opt = m_temp_net_mgr.orig_nodes.back()->GetNetId();
		assert(old_net_id_opt);
		const auto& old_net_category = scene_model.GetNet(*old_net_id_opt)->GetCategory();
		if (end_socket)
		{
			const auto& new_socket_category = end_socket->GetCategory();
			if (old_socket_category == new_socket_category)
			{
				return;
			}

			// either the old one or the new one is not empty, we may need to change the net category
			[[maybe_unused]] model::NetCategory new_category_after_socket_removal = GetNetCategoryExcludingSocket(scene_model, *old_net_id_opt, *old_end_socket->GetUniqueId());
			assert(model::NetCategory::Joinable(new_category_after_socket_removal, new_socket_category));
			if (new_category_after_socket_removal.IsEmpty())
			{
				new_category_after_socket_removal = new_socket_category;
			}
			if (new_category_after_socket_removal != old_net_category)
			{
				request.changed_net_categories.push_back(
					NetModificationRequest::NetCategoryChange{ .net_id = *old_net_id_opt, .new_category = new_category_after_socket_removal });
			}
		}
		else if (!old_socket_category.IsEmpty()) // no end socket, we just disconnected the old one
		{
			// we may need to change the net category
			model::NetCategory new_category_after_socket_removal = GetNetCategoryExcludingSocket(scene_model, *old_net_id_opt, *old_end_socket->GetUniqueId());
			if (new_category_after_socket_removal != old_net_category)
			{
				request.changed_net_categories.push_back(
					NetModificationRequest::NetCategoryChange{ .net_id = *old_net_id_opt, .new_category = new_category_after_socket_removal });
			}
		}
	}
	else if (end_socket)
	{
		// we  a new connection, see if we need to change the category
		auto old_net_id_opt = m_temp_net_mgr.orig_nodes.back()->GetNetId();
		assert(old_net_id_opt);
		const auto& old_net_category = scene_model.GetNet(*old_net_id_opt)->GetCategory();
		const auto& new_socket_category = end_socket->GetCategory();
		if (!new_socket_category.IsEmpty() && new_socket_category != old_net_category)
		{
			assert(model::NetCategory::Joinable(new_socket_category, old_net_category));
			request.changed_net_categories.push_back(
				NetModificationRequest::NetCategoryChange{ .net_id = *old_net_id_opt, .new_category = new_socket_category });
		}
	}
}

void node::logic::LeafNetNodeDragLogic::ResetNodes()
{
	m_temp_net_mgr.ResetNodes();
}

void node::logic::LeafNetNodeDragLogic::PositionNodes(const model::Point& target_point)
{
	BlockSocketObject* end_socket = GetSocketAt(target_point);
	NetSegment* end_segment = nullptr;
	if (!end_socket)
	{
		ResetNodes();
		end_segment = GetSegmentAt(target_point);
	}

	auto end_point = target_point;
	if (end_socket)
	{
		end_point = end_socket->GetCenterInSpace();
	}
	if (end_segment)
	{
		auto segment_center = end_segment->GetCenter();
		if (end_segment->GetOrientation() == model::NetSegmentOrientation::vertical)
		{
			end_point.x = segment_center.x;
		}
		else
		{
			end_point.y = segment_center.y;
		}
	}

	NetSolutionEndDescription end_descriptor{ end_point, {true, true, true, true} };
	if (end_socket)
	{
		std::array<bool, 4> sides{};
		sides[static_cast<int>(end_socket->GetConnectionSide())] = true;
		end_descriptor = NetSolutionEndDescription{ end_point, sides };
	}
	if (end_segment)
	{
		if (end_segment->GetOrientation() == model::NetSegmentOrientation::vertical)
		{
			end_descriptor.allowed_sides = { false, true, false, true };
		}
		else
		{
			end_descriptor.allowed_sides = { true, false ,true, false };
		}
	}

	m_temp_net_mgr.PositionNodes(end_descriptor);
}

void node::logic::LeafNetNodeDragLogic::CleanUp()
{
	m_temp_net_mgr.CleanUp();
}

node::NetModificationRequest node::logic::LeafNetNodeDragLogic::PopulateResultNet(const model::Point& current_mouse_point)
{
	using model::NetNodeId;
	using model::NetSegmentId;
	using enum model::ConnectedSegmentSide;

	const auto* end_socket = GetSocketAt(current_mouse_point);
	NetSegment* end_segment = nullptr;
	if (!end_socket)
	{
		end_segment = GetSegmentAt(current_mouse_point);
	}

	NetsSolver solver;

	model::Point start = std::visit(AnchorStart{}, m_temp_net_mgr.start_anchor);

	solver.SetStartDescription(NetSolutionEndDescription{ start, std::visit(AnchorGetConnectionSide{}, m_temp_net_mgr.start_anchor) });
	if (end_socket)
	{
		std::array<bool, 4> sides{};
		sides[static_cast<int>(end_socket->GetConnectionSide())] = true;
		solver.SetEndDescription(NetSolutionEndDescription{ end_socket->GetCenterInSpace(), sides });
	}
	else if (end_segment)
	{
		if (end_segment->GetOrientation() == model::NetSegmentOrientation::vertical)
		{
			model::Point end_point = current_mouse_point;
			end_point.x = end_segment->GetCenter().x;
			solver.SetEndDescription({ end_point,{ false, true, false, true } });
		}
		else
		{
			model::Point end_point = current_mouse_point;
			end_point.y = end_segment->GetCenter().y;
			solver.SetEndDescription({ end_point,{ true, false, true, false} });
		}
	}
	else
	{
		solver.SetEndDescription(NetSolutionEndDescription{ current_mouse_point, {true, true, true, true} });
	}
	const auto solution = solver.Solve();
	auto report = MakeModificationsReport(solution, m_temp_net_mgr.orig_nodes, m_temp_net_mgr.orig_segments);
	if (end_segment)
	{
		UpdateModificationEndWithSegment(m_temp_net_mgr.orig_nodes, report.request, report.end_node_info, end_segment);
		AddNetModificationsWithSegment(report.request, *end_segment);
	}
	else
	{
		UpdateModificationEndWithSocket(m_temp_net_mgr.orig_nodes, report, end_socket);
		AddNetModificationsWithSocket(report.request, end_socket);
	}

	// fix added nodes to the correct net id
	auto net_id_opt = m_temp_net_mgr.orig_nodes.back()->GetNetId();
	assert(net_id_opt);
	for (auto& added_node : report.request.added_nodes)
	{
		added_node.net_id = *net_id_opt;
		added_node.net_type = NetModificationRequest::IdType::existing_id;
	}
	return std::move(report.request);
}

