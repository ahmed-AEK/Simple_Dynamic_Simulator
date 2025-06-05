#include "HoverHighlightObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObject.hpp"

node::model::Rect node::HoverHighlightObject::RectForObject(const model::Rect& rect)
{
	model::Rect resizer_rect = rect;
	resizer_rect.x -= 5;
	resizer_rect.y -= 5;
	resizer_rect.w += 10;
	resizer_rect.h += 10;
	return resizer_rect;
}

node::HoverHighlightObject::HoverHighlightObject(GraphicsObject& parent_object, GraphicsObjectsManager* manager, const model::ObjectSize& size)
	:ObjectAttachment{size, ObjectType::interactive, nullptr}, m_parent_object{parent_object.GetMIHandlePtr()}, m_manager{manager}
{
	UNUSED_PARAM(m_manager); // silence clang
	SetAligned(false);
}

void node::HoverHighlightObject::OnAttachObject(GraphicsObject& object)
{
	auto new_rect = RectForObject(object.GetSceneRect());
	SetPosition({ new_rect.x, new_rect.y });
	SetSize({ new_rect.w, new_rect.h });
	SetVisible(true);
}

void node::HoverHighlightObject::OnDetachObject()
{
	SetVisible(false);
}

void node::HoverHighlightObject::OnObjectRectUpdate(const model::Rect& rect)
{
	auto new_rect = RectForObject(rect);
	SetPosition({ new_rect.x, new_rect.y });
	SetSize({ new_rect.w, new_rect.h });
}

void node::HoverHighlightObject::Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer)
{
	if (!IsVisible())
	{
		return;
	}
	const SDL_Color outline_color = renderer.GetColor(ColorRole::object_hover_outline);
	SDL_SetRenderDrawColor(renderer, outline_color.r, outline_color.g, outline_color.b, 255);
	SDL_FRect screen_rect = transformer.SpaceToScreenRect(GetInnerRect());
	SDL_RenderRect(renderer, &screen_rect);
	screen_rect.x += 1;
	screen_rect.y += 1;
	screen_rect.w -= 2;
	screen_rect.h -= 2;
	SDL_RenderRect(renderer, &screen_rect);
}

node::GraphicsObject* node::HoverHighlightObject::OnGetInteractableAtPoint(const model::Point& point)
{
	UNUSED_PARAM(point);
	return nullptr; // not hittable
}

node::model::Rect node::HoverHighlightObject::GetInnerRect()
{
	model::Rect draw_area = GetSize().ToRect();
	return draw_area;
}

