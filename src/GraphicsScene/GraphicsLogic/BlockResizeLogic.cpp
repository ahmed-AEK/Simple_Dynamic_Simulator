#include "BlockResizeLogic.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockResizeObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsObjectsManager.hpp"
#include "GraphicsLogic/Anchors.hpp"
#include "GraphicsScene/SolverUtils.hpp"

static void FillSockets(std::vector<node::model::BlockSocketModel>& sockets, node::BlockObject& object)
{
	using namespace node;
	auto&& sockets_vec = object.GetSockets();
	sockets.reserve(sockets_vec.size());
	for (auto&& socket : sockets_vec)
	{
		if (auto id = socket->GetId())
		{
			sockets.push_back(model::BlockSocketModel{ socket->GetSocketType(), *id });
		}
	}
}

static void SetBlockToRect(node::BlockObject& block, const node::model::Rect& new_rect, std::vector<node::model::BlockSocketModel>& temp_sockets)
{
	using namespace node;
	auto&& styler = block.GetStyler();
	styler.PositionSockets(temp_sockets, new_rect, block.GetOrienation());
	for (auto&& socket : temp_sockets)
	{
		auto sock = block.GetSocketById(socket.GetId());
		if (sock)
		{
			sock->SetCenterInBlock(socket.GetPosition());
		}
	}
	block.SetPosition({ new_rect.x, new_rect.y });
	block.SetSize({ new_rect.w, new_rect.h });
}

std::unique_ptr<node::logic::BlockResizeLogic> node::logic::BlockResizeLogic::TryCreate(BlockObject& block,
	BlockResizeObject& resize_object, model::Point drag_start_point, DragSide side, GraphicsScene* scene, 
	GraphicsObjectsManager* manager)
{
	std::unordered_map<model::SocketId, TemporaryNetManager> socket_nets;
	assert(scene);
	for (const auto& socket : block.GetSockets())
	{
		assert(socket->GetId());
		if (!socket->GetId())
		{
			return nullptr;
		}
		auto* current_node = socket->GetConnectedNode();
		if (!current_node)
		{
			continue;
		}
		TemporaryNetManager net = TemporaryNetManager::CreateFromLeafNodeNet(*current_node, *scene);
		net.SetHighlight(false);
		socket_nets[*socket->GetId()] = std::move(net);
	}

	return std::make_unique<BlockResizeLogic>(block, resize_object, drag_start_point, side, 
		scene, manager, std::move(socket_nets));
}

node::logic::BlockResizeLogic::BlockResizeLogic(BlockObject& block, BlockResizeObject& resize_object,
	model::Point drag_start_point, DragSide side, GraphicsScene* scene, GraphicsObjectsManager* manager,
	std::unordered_map<model::SocketId, TemporaryNetManager>&& socket_nets)
	:GraphicsLogic{scene, manager},  m_block{block}, m_resizeObject{resize_object},
	m_initial_rect{block.GetPosition().x, block.GetPosition().y, block.GetSize().w, block.GetSize().h}, 
	m_drag_side{side}, m_socket_nets{std::move(socket_nets)}
{
	assert(manager);
	m_drag_start_point = GetScene()->QuantizePoint(drag_start_point);
	FillSockets(m_temp_sockets, block);
	SetBlockToRect(block, block.GetSceneRect(), m_temp_sockets);

	PositionNodes(block.GetScenePosition(), m_temp_sockets);
}

void node::logic::BlockResizeLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	if (!m_block || !m_resizeObject)
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return;
	}

	model::Point clipped_point = GetScene()->QuantizePoint(current_mouse_point);
	model::Rect new_rect = m_initial_rect;
	switch (m_drag_side)
	{
	case DragSide::LeftTop:
	{
		model::Point lower_right{ new_rect.x + new_rect.w, new_rect.y + new_rect.h };
		clipped_point.x = lower_right.x - clipped_point.x < min_dim ? (lower_right.x - min_dim) : clipped_point.x;
		clipped_point.y = lower_right.y - clipped_point.y < min_dim ? (lower_right.y - min_dim) : clipped_point.y;
		new_rect.w = new_rect.x - clipped_point.x + new_rect.w;
		new_rect.h = new_rect.y - clipped_point.y + new_rect.h;
		new_rect.x = clipped_point.x;
		new_rect.y = clipped_point.y;
		break;
	}
	case DragSide::RightTop:
	{
		model::Point lower_left{ new_rect.x, new_rect.y + new_rect.h };
		clipped_point.x = clipped_point.x - lower_left.x < min_dim ? lower_left.x + min_dim : clipped_point.x;
		clipped_point.y = lower_left.y - clipped_point.y < min_dim ? lower_left.y - min_dim : clipped_point.y;
		new_rect.w = clipped_point.x - new_rect.x;
		new_rect.h = new_rect.y - clipped_point.y + new_rect.h;
		new_rect.y = clipped_point.y;
		break;
	}
	case DragSide::LeftBottom:
	{
		model::Point right_top{ new_rect.x + new_rect.w, new_rect.y };
		clipped_point.x = right_top.x - clipped_point.x < min_dim ? right_top.x - min_dim : clipped_point.x;
		clipped_point.y = clipped_point.y - right_top.y < min_dim ? right_top.y + min_dim : clipped_point.y;
		new_rect.w = new_rect.x - clipped_point.x + new_rect.w;
		new_rect.h = clipped_point.y - new_rect.y;
		new_rect.w = std::max(new_rect.w, min_dim);
		new_rect.h = std::max(new_rect.h, min_dim);
		new_rect.x = clipped_point.x;
		break;
	}
	case DragSide::RightBottom:
	{
		model::Point left_top{ new_rect.x, new_rect.y };
		clipped_point.x = clipped_point.x - left_top.x < min_dim ? left_top.x + min_dim : clipped_point.x;
		clipped_point.y = clipped_point.y - left_top.y < min_dim ? left_top.y + min_dim : clipped_point.y;
		new_rect.w = clipped_point.x - new_rect.x;
		new_rect.h = clipped_point.y - new_rect.y;
		new_rect.w = std::max(new_rect.w, min_dim);
		new_rect.h = std::max(new_rect.h, min_dim);
		break;
	}
	}

	auto&& block = *m_block.GetObjectPtr();
	SetBlockToRect(block, new_rect, m_temp_sockets);
	auto new_object_rect = BlockResizeObject::RectForBlockRect(new_rect);
	m_resizeObject->SetPosition({ new_object_rect.x, new_object_rect.y });
	m_resizeObject->SetSize({ new_object_rect.w, new_object_rect.h });
	PositionNodes({ new_rect.x, new_rect.y }, m_temp_sockets);
}

void node::logic::BlockResizeLogic::OnCancel()
{
	CleanUp();
}

MI::ClickEvent node::logic::BlockResizeLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	if (!m_block || !m_resizeObject)
	{
		CleanUp();
		return MI::ClickEvent::CAPTURE_END;
	}
	auto&& block = *m_block.GetObjectPtr();
	if (!block.GetModelId())
	{
		CleanUp();
		return MI::ClickEvent::CAPTURE_END;
	}

	model::Point clipped_point = GetScene()->QuantizePoint(current_mouse_point);
	model::Rect new_rect = m_initial_rect;
	switch (m_drag_side)
	{
	case DragSide::LeftTop:
	{
		model::Point lower_right{ new_rect.x + new_rect.w, new_rect.y + new_rect.h };
		clipped_point.x = lower_right.x - clipped_point.x < min_dim ? (lower_right.x - min_dim) : clipped_point.x;
		clipped_point.y = lower_right.y - clipped_point.y < min_dim ? (lower_right.y - min_dim) : clipped_point.y;
		new_rect.w = new_rect.x - clipped_point.x + new_rect.w;
		new_rect.h = new_rect.y - clipped_point.y + new_rect.h;
		new_rect.x = clipped_point.x;
		new_rect.y = clipped_point.y;
		break;
	}
	case DragSide::RightTop:
	{
		model::Point lower_left{ new_rect.x, new_rect.y + new_rect.h };
		clipped_point.x = clipped_point.x - lower_left.x < min_dim ? lower_left.x + min_dim : clipped_point.x;
		clipped_point.y = lower_left.y - clipped_point.y < min_dim ? lower_left.y - min_dim : clipped_point.y;
		new_rect.w = clipped_point.x - new_rect.x;
		new_rect.h = new_rect.y - clipped_point.y + new_rect.h;
		new_rect.y = clipped_point.y;
		break;
	}
	case DragSide::LeftBottom:
	{
		model::Point right_top{ new_rect.x + new_rect.w, new_rect.y };
		clipped_point.x = right_top.x - clipped_point.x < min_dim ? right_top.x - min_dim : clipped_point.x;
		clipped_point.y = clipped_point.y - right_top.y < min_dim ? right_top.y + min_dim : clipped_point.y;
		new_rect.w = new_rect.x - clipped_point.x + new_rect.w;
		new_rect.h = clipped_point.y - new_rect.y;
		new_rect.w = std::max(new_rect.w, min_dim);
		new_rect.h = std::max(new_rect.h, min_dim);
		new_rect.x = clipped_point.x;
		break;
	}
	case DragSide::RightBottom:
	{
		model::Point left_top{new_rect.x, new_rect.y};
		clipped_point.x = clipped_point.x - left_top.x < min_dim ? left_top.x + min_dim : clipped_point.x;
		clipped_point.y = clipped_point.y - left_top.y < min_dim ? left_top.y + min_dim : clipped_point.y;	
		new_rect.w = clipped_point.x - new_rect.x;
		new_rect.h = clipped_point.y - new_rect.y;
		new_rect.w = std::max(new_rect.w, min_dim);
		new_rect.h = std::max(new_rect.h, min_dim);
		break;
	}
	}

	SetBlockToRect(block, new_rect, m_temp_sockets);

	auto id = *block.GetModelId();
	auto new_sockets = m_temp_sockets;
	auto orientation = block.GetOrienation();
	CleanUp();

	NetModificationRequest main_request;
	auto& socket_objects = block.GetSockets();
	for (size_t i = 0; i < socket_objects.size(); i++)
	{
		auto* end_socket = socket_objects[i].get();
		auto* connected_node = end_socket->GetConnectedNode();
		if (!connected_node)
		{
			continue;
		}
		auto new_socket_pos = new_sockets[i].GetPosition();

		auto branch = GetNetBranchForLeafNode(*connected_node);
		std::reverse(branch.nodes.begin(), branch.nodes.end());
		std::reverse(branch.segments.begin(), branch.segments.end());

		NetsSolver solver;

		model::Point start = branch.nodes[0]->getCenter();
		auto start_anchor = logic::CreateStartAnchor(branch.nodes, branch.segments);

		solver.SetStartDescription(NetSolutionEndDescription{ start, std::visit(AnchorGetConnectionSide{}, start_anchor) });

		std::array<bool, 4> sides{};
		sides[static_cast<int>(new_sockets[i].GetConnectionSide())] = true;
		solver.SetEndDescription(NetSolutionEndDescription{ model::Point{new_rect.x, new_rect.y} + new_socket_pos, sides });

		const auto solution = solver.Solve();
		auto report = MakeModificationsReport(solution, branch.nodes, branch.segments);
		UpdateModificationEndWithSocket(branch.nodes, report, end_socket);
		auto net_id_opt = branch.nodes.back()->GetNetId();
		assert(net_id_opt);
		for (auto& added_node : report.request.added_nodes)
		{
			added_node.net_id = *net_id_opt;
			added_node.net_type = NetModificationRequest::IdType::existing_id;
		}
		MergeModificationRequests(report.request, main_request);
	}

	GetObjectsManager()->GetSceneModel()->ResizeBlockById(id, new_rect, orientation, 
		new_sockets, std::move(main_request));

	return MI::ClickEvent::CAPTURE_END;
}

void node::logic::BlockResizeLogic::CleanUp()
{
	if (auto ptr = m_block.GetObjectPtr())
	{
		auto&& block = *ptr;
		SetBlockToRect(block, m_initial_rect, m_temp_sockets);
	}

	m_block.GetObjectPtr()->SetPosition({ m_initial_rect.x, m_initial_rect.y });
	m_block.GetObjectPtr()->SetSize({ m_initial_rect.w, m_initial_rect.h });

	if (auto ptr = m_resizeObject.GetObjectPtr())
	{
		auto new_rect = BlockResizeObject::RectForBlockRect(m_initial_rect);
		ptr->SetPosition({ new_rect.x, new_rect.y });
		ptr->SetSize({ new_rect.w, new_rect.h });
	}
	for (auto&& [id, net] : m_socket_nets)
	{
		net.CleanUp();
	}
}

void node::logic::BlockResizeLogic::PositionNodes(const model::Point& edge, 
	std::span<const model::BlockSocketModel> sockets)
{
	for (auto&& [id, net] : m_socket_nets)
	{
		auto* socket = m_block->GetSocketById(id);

		auto socket_it = std::find_if(sockets.begin(), sockets.end(), [&](const model::BlockSocketModel& socket_model)
			{
				return socket_model.GetId() == id;
			});
		assert(socket_it != sockets.end());

		auto end_point = edge + socket_it->GetPosition();

		std::array<bool, 4> sides{};
		sides[static_cast<int>(socket->GetConnectionSide())] = true;

		net.PositionNodes(NetSolutionEndDescription{ end_point, sides });
	}
}



