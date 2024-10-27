#include "BlockResizeObject.hpp"
#include "GraphicsScene.hpp"
#include "BlockObject.hpp"
#include "GraphicsLogic/BlockResizeLogic.hpp"
#include "GraphicsLogic/BlockRotateLogic.hpp"

node::model::Rect node::BlockResizeObject::RectForBlockRect(const model::Rect& rect)
{
	model::Rect resizer_rect = rect;
	resizer_rect.x -= 5;
	resizer_rect.y -= 41;
	resizer_rect.w += 10;
	resizer_rect.h += 10 + 36;
	return resizer_rect;
}

node::BlockResizeObject::BlockResizeObject(HandlePtr<GraphicsObject> parent_block, GraphicsObjectsManager* manager, model::Rect sceneRect, GraphicsScene* scene)
	:GraphicsObject{sceneRect, ObjectType::interactive, scene}, m_parent_block{parent_block}, m_manager{manager}, m_rotate_rasterizer{"assets/redo.svg",0,0}
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
	rotate_button.h = 36;
	rotate_button.x = rotate_button.x + rotate_button.w / 2 - 18;
	rotate_button.w = 36;
	SDL_Rect rotate_btn_screen = transformer.SpaceToScreenRect(rotate_button);
	SDL_RenderFillRect(renderer, &rotate_btn_screen);

	rotate_btn_screen.x += 2;
	rotate_btn_screen.y += 2;
	rotate_btn_screen.w -= 4;
	rotate_btn_screen.h -= 4;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &rotate_btn_screen);

	rotate_btn_screen.x += 1;
	rotate_btn_screen.y += 1;
	rotate_btn_screen.w -= 2;
	rotate_btn_screen.h -= 2;
	m_rotate_rasterizer.SetSize(rotate_btn_screen.w, rotate_btn_screen.h);
	m_rotate_rasterizer.Draw(renderer, rotate_btn_screen.x, rotate_btn_screen.y);
	
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
	rotate_button.h = 36;
	rotate_button.x = rotate_button.x + rotate_button.w / 2 - 18;
	rotate_button.w = 36;
	SDL_Rect rotate_button_sdl{ node::ToSDLRect(rotate_button) };
	if (SDL_PointInRect(&point_sdl, &rotate_button_sdl))
	{
		return this;
	}

	return nullptr;
}

MI::ClickEvent node::BlockResizeObject::OnLMBDown(MouseButtonEvent& e)
{
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
	SDL_Point point_sdl{ e.point.x, e.point.y };

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
	rotate_button.h = 36;
	rotate_button.x = rotate_button.x + rotate_button.w / 2 - 18;
	rotate_button.w = 36;
	SDL_Rect rotate_button_sdl{ node::ToSDLRect(rotate_button) };
	if (SDL_PointInRect(&point_sdl, &rotate_button_sdl))
	{
		rotate = true;
	}

	if (side)
	{
		auto logic_obj = std::make_unique<logic::BlockResizeLogic>(static_cast<BlockObject&>(*m_parent_block.GetObjectPtr()), *this, model::Point{e.point.x, e.point.y}, *side, scene, m_manager);
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
	draw_area.y += 36;
	draw_area.h -= 36;
	return draw_area;
}

