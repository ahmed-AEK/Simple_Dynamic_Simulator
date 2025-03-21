#include "BlockRotateLogic.hpp"
#include "BlockObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/SolverUtils.hpp"
#include "GraphicsLogic/Anchors.hpp"

node::logic::BlockRotateLogic::BlockRotateLogic(const model::Rect& rotate_btn_rect, BlockObject& block, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_rotate_btn_rect{rotate_btn_rect}, m_block{block}
{
	assert(scene);
	assert(manager);
}

MI::ClickEvent node::logic::BlockRotateLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	SDL_FRect rotate_btn_rect_sdl = node::ToSDLRect(m_rotate_btn_rect);
	SDL_FPoint mouse_screen = node::ToSDLPoint(current_mouse_point);
	if (!m_block || !SDL_PointInRectFloat(&mouse_screen, &rotate_btn_rect_sdl))
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	auto&& block = *m_block.GetObjectPtr();
	if (!block.GetModelId())
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	model::BlockOrientation new_orientation = static_cast<model::BlockOrientation>((static_cast<int>(block.GetOrienation()) + 1) % 4);
	auto&& styler = block.GetStyler();
	auto* block_model_ref = GetObjectsManager()->GetSceneModel()->GetModel().GetBlockById(*block.GetModelId());
	if (!block_model_ref)
	{
		return MI::ClickEvent::CAPTURE_END;
	}
	std::vector<model::BlockSocketModel> new_sockets;
	{
		auto sockets_span = block_model_ref->GetSockets();
		new_sockets = std::vector<model::BlockSocketModel>{ sockets_span.begin(), sockets_span.end() };
	} 
	model::Rect new_rect = block_model_ref->GetBounds();
	model::Point center_point{ new_rect.x + new_rect.w / 2, new_rect.y + new_rect.h / 2 };
	std::swap(new_rect.w, new_rect.h);
	new_rect.x = center_point.x - new_rect.w / 2;
	new_rect.y = center_point.y - new_rect.h / 2;
	styler.PositionSockets(new_sockets, new_rect, new_orientation);

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
		solver.SetEndDescription(NetSolutionEndDescription{ model::Point{new_rect.x, new_rect.y} + new_socket_pos, sides});

		const auto solution = solver.Solve();
		auto report = MakeModificationsReport(solution, branch.nodes, branch.segments);
		UpdateModificationEndWithSocket(branch.nodes, report, end_socket);
		MergeModificationRequests(report.request, main_request);
	}
	GetObjectsManager()->GetSceneModel()->ResizeBlockById(*block.GetModelId(), new_rect, new_orientation, new_sockets, std::move(main_request));
	return MI::ClickEvent::CAPTURE_END;
}

