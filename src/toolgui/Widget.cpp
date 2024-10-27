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

node::Widget* node::Widget::GetFocusable()
{
    if (IsFocusable())
    {
        return this;
    }
    return nullptr;
}

node::Application* node::Widget::GetApp() const
{
    if (auto parent = GetParent())
    {
        return parent->GetApp();
    }
    return nullptr;
}

void node::Widget::OnSetRect(const SDL_Rect &rect)
{
    WidgetMouseInteractable::SetRectImpl(rect);
}

node::Widget::~Widget()
{

}

node::Widget::Widget(const SDL_Rect& rect, Widget* parent)
:m_parent(parent), m_rect_base(rect)
{
    WidgetMouseInteractable::SetRectImpl(rect);
}

node::Widget* node::Widget::OnGetInteractableAtPoint(const SDL_Point &point)
{
    UNUSED_PARAM(point);
    return this;
}
