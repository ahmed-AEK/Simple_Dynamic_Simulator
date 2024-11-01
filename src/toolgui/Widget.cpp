#include "Widget.hpp"
#include "Scene.hpp"
#include "toolgui/ContextMenu.hpp"
#include "toolgui/MouseInteractable.hpp"

template class MI::MouseInteractable<node::Widget, SDL_FRect, SDL_FPoint>;

void node::Widget::SetRect(const SDL_FRect& rect)
{
    OnSetRect(rect);
};

bool node::Widget::OnScroll(const double amount, const SDL_FPoint& p)
{
    UNUSED_PARAM(amount);
    UNUSED_PARAM(p);
    return false;
}

const SDL_FRect& node::Widget::GetBaseRect() noexcept
{
    return m_rect_base;
}

void node::Widget::SetBaseRect(const SDL_FRect& rect) noexcept
{
    m_rect_base = rect;
}

node::Widget* node::Widget::GetFocusable()
{
    if (IsFocusable())
    {
        return this;
    }
    if (m_focus_proxy)
    {
        return m_focus_proxy->GetFocusable();
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

void node::Widget::SetFocusProxy(Widget* other)
{
    if (!other)
    {
        m_focus_proxy = nullptr;
        return;
    }
    m_focus_proxy = other->GetMIHandlePtr();
}

void node::Widget::OnSetRect(const SDL_FRect &rect)
{
    WidgetMouseInteractable::SetRectImpl(rect);
}

node::Widget::~Widget()
{

}

node::Widget::Widget(const SDL_FRect& rect, Widget* parent)
:m_parent(parent), m_rect_base(rect)
{
    WidgetMouseInteractable::SetRectImpl(rect);
}

node::Widget* node::Widget::OnGetInteractableAtPoint(const SDL_FPoint &point)
{
    UNUSED_PARAM(point);
    return this;
}
