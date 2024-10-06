#include "BlockResizeObject.hpp"
#include "GraphicsScene.hpp"
#include "BlockObject.hpp"
#include "GraphicsLogic/BlockResizeLogic.hpp"
#include "GraphicsLogic/BlockRotateLogic.hpp"

node::model::Rect node::BlockResizeObject::RectForBlockRect(const model::Rect& rect)
{
	model::Rect resizer_rect = rect;
	resizer_rect.x -= 5;
	resizer_rect.y -= 35;
	resizer_rect.w += 10;
	resizer_rect.h += 10 + 30;
	return resizer_rect;
}

node::BlockResizeObject::BlockResizeObject(HandlePtr<GraphicsObject> parent_block, GraphicsObjectsManager* manager, model::Rect sceneRect, GraphicsScene* scene)
	:GraphicsObject{sceneRect, ObjectType::interactive, scene}, m_parent_block{parent_block}, m_manager{manager}
{
	b_aligned = false;
}

void node::BlockResizeObject::Draw(SDL_Renderer* renderer, const SpaceScreenTransformer& transformer)
{
	if (!IsVisible())
	{
		return;
	}
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_Rect screen_rect = transformer.SpaceToScreenRect(GetInnerRect());
	SDL_RenderDrawRect(renderer, &screen_rect);

	const int width = transformer.SpaceToScreenVector({ corner_width,corner_width }).x;
	
	SDL_Rect rect1{ screen_rect.x, screen_rect.y, width, width };
	SDL_RenderFillRect(renderer, &rect1);

	SDL_Rect rect2{ screen_rect.x, screen_rect.y + screen_rect.h - width, width, width };
	SDL_RenderFillRect(renderer, &rect2);

	SDL_Rect rect3{ screen_rect.x + screen_rect.w - width, screen_rect.y, width, width };
	SDL_RenderFillRect(renderer, &rect3);

	SDL_Rect rect4{ screen_rect.x + screen_rect.w - width, screen_rect.y + screen_rect.h - width, width, width };
	SDL_RenderFillRect(renderer, &rect4);

	model::Rect rotate_button = GetSpaceRect();
	rotate_button.h = 30;
	rotate_button.x = rotate_button.x + rotate_button.w / 2 - 15;
	rotate_button.w = 30;
	SDL_Rect rotate_btn_screen = transformer.SpaceToScreenRect(rotate_button);
	SDL_RenderFillRect(renderer, &rotate_btn_screen);
	
}

node::GraphicsObject* node::BlockResizeObject::OnGetInteractableAtPoint(const model::Point& point)
{
	auto* scene = GetScene();
	assert(scene);
	if (!scene || !IsVisible())
	{
		return nullptr;
	}

	model::Rect rect = GetInnerRect();
	SDL_Point point_sdl{ point.x, point.y };

	SDL_Rect rect1{ rect.x, rect.y, corner_width, corner_width };
	if (SDL_PointInRect(&point_sdl, &rect1))
	{
		return this;
	}

	SDL_Rect rect2{ rect.x, rect.y + rect.h - corner_width, corner_width, corner_width };
	if (SDL_PointInRect(&point_sdl, &rect2))
	{
		return this;
	}

	SDL_Rect rect3{ rect.x + rect.w - corner_width, rect.y, corner_width, corner_width };
	if (SDL_PointInRect(&point_sdl, &rect3))
	{
		return this;
	}

	SDL_Rect rect4{ rect.x + rect.w - corner_width, rect.y + rect.h - corner_width, corner_width, corner_width };
	if (SDL_PointInRect(&point_sdl, &rect4))
	{
		return this;
	}

	model::Rect rotate_button = GetSpaceRect();
	rotate_button.h = 30;
	rotate_button.x = rotate_button.x + rotate_button.w / 2 - 15;
	rotate_button.w = 30;
	SDL_Rect rotate_button_sdl{ node::ToSDLRect(rotate_button) };
	if (SDL_PointInRect(&point_sdl, &rotate_button_sdl))
	{
		return this;
	}

	return nullptr;
}

MI::ClickEvent node::BlockResizeObject::OnLMBDown(const model::Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	if (!m_parent_block)
	{
		return MI::ClickEvent::NONE;
	}

	auto* scene = GetScene();
	assert(scene);
	if (!scene)
	{
		return MI::ClickEvent::NONE;
	}
	
	std::optional<logic::BlockResizeLogic::DragSide> side;
	bool rotate = false;

	model::Rect rect = GetInnerRect();
	SDL_Point point_sdl{ current_mouse_point.x, current_mouse_point.y };

	SDL_Rect rect1{ rect.x, rect.y, corner_width, corner_width };
	if (SDL_PointInRect(&point_sdl, &rect1))
	{
		side = logic::BlockResizeLogic::DragSide::LeftTop;
	}

	SDL_Rect rect2{ rect.x, rect.y + rect.h - corner_width, corner_width, corner_width };
	if (SDL_PointInRect(&point_sdl, &rect2))
	{
		side = logic::BlockResizeLogic::DragSide::LeftBottom;
	}

	SDL_Rect rect3{ rect.x + rect.w - corner_width, rect.y, corner_width, corner_width };
	if (SDL_PointInRect(&point_sdl, &rect3))
	{
		side = logic::BlockResizeLogic::DragSide::RightTop;
	}

	SDL_Rect rect4{ rect.x + rect.w - corner_width, rect.y + rect.h - corner_width, corner_width, corner_width };
	if (SDL_PointInRect(&point_sdl, &rect4))
	{
		side = logic::BlockResizeLogic::DragSide::RightBottom;
	}

	model::Rect rotate_button = GetSpaceRect();
	rotate_button.h = 30;
	rotate_button.x = rotate_button.x + rotate_button.w / 2 - 15;
	rotate_button.w = 30;
	SDL_Rect rotate_button_sdl{ node::ToSDLRect(rotate_button) };
	if (SDL_PointInRect(&point_sdl, &rotate_button_sdl))
	{
		rotate = true;
	}

	if (side)
	{
		auto logic_obj = std::make_unique<logic::BlockResizeLogic>(static_cast<BlockObject&>(*m_parent_block.GetObjectPtr()), *this, current_mouse_point, *side, scene, m_manager);
		SDL_Log("Clicked On Resize Object!");
		scene->SetGraphicsLogic(std::move(logic_obj));
		return MI::ClickEvent::CAPTURE_START;
	}
	
	if (rotate)
	{
		auto logic_obj = std::make_unique<logic::BlockRotateLogic>(rotate_button, static_cast<BlockObject&>(*m_parent_block.GetObjectPtr()), scene, m_manager);
		SDL_Log("Clicked On Rotate Object!");
		scene->SetGraphicsLogic(std::move(logic_obj));
		return MI::ClickEvent::CAPTURE_START;
	}
	return MI::ClickEvent::NONE;

	
}

node::model::Rect node::BlockResizeObject::GetInnerRect()
{
	model::Rect draw_area = GetSpaceRect();
	draw_area.y += 30;
	draw_area.h -= 30;
	return draw_area;
}

