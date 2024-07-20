#include "Widget.hpp"
#include "Scene.hpp"
#include "toolgui/ContextMenu.hpp"

namespace node
{

    void Widget::SetRect(const SDL_Rect& rect)
    {
        OnSetRect(rect);
    };

    bool Widget::OnScroll(const double amount, const SDL_Point& p)
    {
        UNUSED_PARAM(amount);
        UNUSED_PARAM(p);
        return false;
    }
    const SDL_Rect& Widget::GetBaseRect() noexcept
    {
        return m_rect_base;
    }
    void Widget::OnSetRect(const SDL_Rect &rect)
    {
        WidgetMouseInteractable::SetRectImpl(rect);
    }

    Widget::~Widget()
    {

    }

    Widget::Widget(SDL_Rect rect, Scene* parent)
    :p_parent(parent), m_rect_base(rect)
    {
        WidgetMouseInteractable::SetRectImpl(rect);
    }
    void Widget::InvalidateRect()
    {
        p_parent->InvalidateRect();
    }
    WidgetScaling Widget::GetScalingType() const
    {
        return m_scalingType;
    }
    node::Widget* Widget::OnGetInteractableAtPoint(const SDL_Point &point)
    {
        UNUSED_PARAM(point);
        return this;
    }
    node::Scene* Widget::GetScene() const noexcept
    {
        return p_parent;
    }



}