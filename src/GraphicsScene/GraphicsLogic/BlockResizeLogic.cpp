#include "BlockResizeLogic.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockResizeObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "NodeSDLStylers/BlockStyler.hpp"
#include "GraphicsObjectsManager.hpp"

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
	block.SetSpaceRect(new_rect);
}

node::logic::BlockResizeLogic::BlockResizeLogic(BlockObject& block, BlockResizeObject& resize_object, 
	model::Point drag_start_point, DragSide side, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager},  m_block{block.GetMIHandlePtr()}, m_resizeObject{resize_object.GetMIHandlePtr()},
	m_initial_rect{block.GetSpaceRect()}, m_drag_side{side}
{
	assert(scene);
	assert(manager);
	m_drag_start_point = GetScene()->QuantizePoint(drag_start_point);
	FillSockets(m_temp_sockets, block);
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

	auto&& block = static_cast<BlockObject&>(*m_block.GetObjectPtr());
	SetBlockToRect(block, new_rect, m_temp_sockets);
	m_resizeObject->SetSpaceRect(BlockResizeObject::RectForBlockRect(new_rect));

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
	auto&& block = static_cast<BlockObject&>(*m_block.GetObjectPtr());
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

	GetObjectsManager()->GetSceneModel()->ResizeBlockById(id, new_rect, orientation, new_sockets);

	return MI::ClickEvent::CAPTURE_END;
}

void node::logic::BlockResizeLogic::CleanUp()
{
	if (auto ptr = m_block.GetObjectPtr())
	{
		auto&& block = static_cast<BlockObject&>(*ptr);
		SetBlockToRect(block, m_initial_rect, m_temp_sockets);
	}

	m_block.GetObjectPtr()->SetSpaceRect(m_initial_rect);

	if (auto ptr = m_resizeObject.GetObjectPtr())
	{
		ptr->SetSpaceRect(BlockResizeObject::RectForBlockRect(m_initial_rect));
	}
}



