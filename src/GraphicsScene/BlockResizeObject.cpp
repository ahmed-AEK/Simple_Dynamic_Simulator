#include "BlockResizeObject.hpp"
#include "GraphicsScene.hpp"
#include "BlockObject.hpp"

node::BlockResizeObject::BlockResizeObject(HandlePtr<GraphicsObject> parent_block, model::Rect sceneRect, IGraphicsScene* scene)
	:GraphicsObject{sceneRect, ObjectType::interactive, scene}, m_parent_block{parent_block}
{
	b_aligned = false;
}

void node::BlockResizeObject::Draw(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_Rect screen_rect = GetScene()->GetSpaceScreenTransformer().SpaceToScreenRect(GetSpaceRect());
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
	SDL_Rect screen_rect = GetScene()->GetSpaceScreenTransformer().SpaceToScreenRect(GetSpaceRect());
	SDL_Point screen_point = GetScene()->GetSpaceScreenTransformer().SpaceToScreenPoint(point);

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

