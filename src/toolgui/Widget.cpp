#include "Widget.hpp"
#include "Scene.hpp"
#include "toolgui/ContextMenu.hpp"
#include "toolgui/MouseInteractable.hpp"

template class MI::MouseInteractable<node::Widget, SDL_Rect, SDL_Point>;

void node::Widget::SetRect(const SDL_Rect& rect)
{
    OnSetRect(rect);
};

bool node::Widget::OnScroll(const double amount, const SDL_Point& p)
{
    UNUSED_PARAM(amount);
    UNUSED_PARAM(p);
    return false;
}

const SDL_Rect& node::Widget::GetBaseRect() noexcept
{
    return m_rect_base;
}

void node::Widget::SetBaseRect(const SDL_Rect& rect) noexcept
{
    m_rect_base = rect;
}

void node::Widget::OnSetRect(const SDL_Rect &rect)
{
    WidgetMouseInteractable::SetRectImpl(rect);
}

node::Widget::~Widget()
{

}

node::Widget::Widget(const SDL_Rect& rect, Scene* parent)
:p_parent(parent), m_rect_base(rect)
{
    WidgetMouseInteractable::SetRectImpl(rect);
}
void node::Widget::InvalidateRect()
{
    p_parent->InvalidateRect();
}

node::Widget* node::Widget::OnGetInteractableAtPoint(const SDL_Point &point)
{
    UNUSED_PARAM(point);
    return this;
}
node::Scene* node::Widget::GetScene() const noexcept
{
    return p_parent;
}
