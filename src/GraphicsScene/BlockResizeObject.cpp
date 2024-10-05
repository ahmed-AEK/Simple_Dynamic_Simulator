#include "BlockResizeObject.hpp"
#include "GraphicsScene.hpp"
#include "BlockObject.hpp"
#include "GraphicsLogic/BlockResizeLogic.hpp"

node::model::Rect node::BlockResizeObject::RectForBlockRect(const model::Rect& rect)
{
	model::Rect resizer_rect = rect;
	resizer_rect.x -= 5;
	resizer_rect.y -= 5;
	resizer_rect.w += 10;
	resizer_rect.h += 10;
	return resizer_rect;
}

node::BlockResizeObject::BlockResizeObject(HandlePtr<GraphicsObject> parent_block, GraphicsObjectsManager* manager, model::Rect sceneRect, GraphicsScene* scene)
	:GraphicsObject{sceneRect, ObjectType::interactive, scene}, m_parent_block{parent_block}, m_manager{manager}
{
	b_aligned = false;
}

void node::BlockResizeObject::Draw(SDL_Renderer* renderer, const SpaceScreenTransformer& transformer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_Rect screen_rect = transformer.SpaceToScreenRect(GetSpaceRect());
	SDL_RenderDrawRect(renderer, &screen_rect);

	static constexpr int width = 10;

	SDL_Rect rect1{ screen_rect.x, screen_rect.y, width, width };
	SDL_RenderFillRect(renderer, &rect1);

	SDL_Rect rect2{ screen_rect.x, screen_rect.y + screen_rect.h - width, width, width };
	SDL_RenderFillRect(renderer, &rect2);

	SDL_Rect rect3{ screen_rect.x + screen_rect.w - width, screen_rect.y, width, width };
	SDL_RenderFillRect(renderer, &rect3);

	SDL_Rect rect4{ screen_rect.x + screen_rect.w - width, screen_rect.y + screen_rect.h - width, width, width };
	SDL_RenderFillRect(renderer, &rect4);
}

node::GraphicsObject* node::BlockResizeObject::OnGetInteractableAtPoint(const model::Point& point)
{
	auto* scene = GetScene();
	assert(scene);
	if (!scene)
	{
		return nullptr;
	}

	SDL_Rect screen_rect = scene->GetSpaceScreenTransformer().SpaceToScreenRect(GetSpaceRect());
	SDL_Point screen_point = scene->GetSpaceScreenTransformer().SpaceToScreenPoint(point);

	static constexpr int width = 10;

	SDL_Rect rect1{ screen_rect.x, screen_rect.y, width, width };
	if (SDL_PointInRect(&screen_point, &rect1))
	{
		return this;
	}

	SDL_Rect rect2{ screen_rect.x, screen_rect.y + screen_rect.h - width, width, width };
	if (SDL_PointInRect(&screen_point, &rect2))
	{
		return this;
	}

	SDL_Rect rect3{ screen_rect.x + screen_rect.w - width, screen_rect.y, width, width };
	if (SDL_PointInRect(&screen_point, &rect3))
	{
		return this;
	}

	SDL_Rect rect4{ screen_rect.x + screen_rect.w - width, screen_rect.y + screen_rect.h - width, width, width };
	if (SDL_PointInRect(&screen_point, &rect4))
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
	SDL_Rect screen_rect = scene->GetSpaceScreenTransformer().SpaceToScreenRect(GetSpaceRect());
	SDL_Point screen_point = scene->GetSpaceScreenTransformer().SpaceToScreenPoint(current_mouse_point);

	static constexpr int width = 10;

	SDL_Rect rect1{ screen_rect.x, screen_rect.y, width, width };
	if (SDL_PointInRect(&screen_point, &rect1))
	{
		side = logic::BlockResizeLogic::DragSide::LeftTop;
	}

	SDL_Rect rect2{ screen_rect.x, screen_rect.y + screen_rect.h - width, width, width };
	if (SDL_PointInRect(&screen_point, &rect2))
	{
		side = logic::BlockResizeLogic::DragSide::LeftBottom;
	}

	SDL_Rect rect3{ screen_rect.x + screen_rect.w - width, screen_rect.y, width, width };
	if (SDL_PointInRect(&screen_point, &rect3))
	{
		side = logic::BlockResizeLogic::DragSide::RightTop;
	}

	SDL_Rect rect4{ screen_rect.x + screen_rect.w - width, screen_rect.y + screen_rect.h - width, width, width };
	if (SDL_PointInRect(&screen_point, &rect4))
	{
		side = logic::BlockResizeLogic::DragSide::RightBottom;
	}

	if (!side)
	{
		return MI::ClickEvent::NONE;
	}

	auto logic_obj = std::make_unique<logic::BlockResizeLogic>(static_cast<BlockObject&>(*m_parent_block.GetObjectPtr()), *this, current_mouse_point, *side, scene, m_manager);
	SDL_Log("Clicked On Resize Object!");
	scene->SetGraphicsLogic(std::move(logic_obj));

	return MI::ClickEvent::CAPTURE_START;
}

