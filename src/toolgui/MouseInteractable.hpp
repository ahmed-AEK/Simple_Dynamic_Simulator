#pragma once

#include "toolgui/MouseInteractable_interface.hpp"

namespace MI
{

template <typename T, typename Rect, typename Point>
MouseInteractable<T, Rect, Point>::MouseInteractable(MouseInteractable<T, Rect, Point>&& other) noexcept
    : m_rect(other.m_rect), MI_handle_ptr(std::move(other.MI_handle_ptr.m_ptr))
{
    this->MI_handle_ptr.m_ptr->object = static_cast<T*>(this);
}
    
template <typename T, typename Rect, typename Point>
MouseInteractable<T, Rect, Point>& MouseInteractable<T, Rect, Point>::operator=(MouseInteractable<T, Rect, Point>&& other) noexcept
{
    this->m_rect = other.m_rect;
    this->MI_handle_ptr.m_ptr = std::move(other.MI_handle_ptr.m_ptr);
    this->MI_handle_ptr.m_ptr->object = static_cast<T*>(this);
	return *this;
}

template <typename T, typename Rect, typename Point>
T* MouseInteractable<T, Rect, Point>::GetInteractableAtPoint(const Point& point)
{
    SDL_Point SDLpoint{ point.x, point.y };
    SDL_Rect SDLrect{ m_rect.x, m_rect.y, m_rect.w, m_rect.h };
    if (SDL_PointInRect(&SDLpoint, &SDLrect))
    {
        return this->OnGetInteractableAtPoint(point);
    }
    else
    {
        return nullptr;
    }
}

template <typename T, typename Rect, typename Point>
void MouseInteractable<T, Rect, Point>::MouseOut()
{
    this->OnMouseOut();
}

template <typename T, typename Rect, typename Point>
void MouseInteractable<T, Rect, Point>::MouseIn()
{
    this->OnMouseIn();
}

template <typename T, typename Rect, typename Point>
void MouseInteractable<T, Rect, Point>::MouseMove(const Point& current_mouse_point)
{
    this->OnMouseMove(current_mouse_point);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::LMBDown(const Point& current_mouse_point)
{
    return this->OnLMBDown(current_mouse_point);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::RMBDown(const Point& current_mouse_point)
{
    return this->OnRMBDown(current_mouse_point);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::RMBUp(const Point& current_mouse_point)
{
    return this->OnRMBUp(current_mouse_point);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::LMBUp(const Point& current_mouse_point)
{
    return this->OnLMBUp(current_mouse_point);
}

template <typename T, typename Rect, typename Point>
node::HandlePtr<T> MouseInteractable<T, Rect, Point>::GetMIHandlePtr()
{
    return this->MI_handle_ptr.GetHandlePtr();
}

template <typename T, typename Rect, typename Point>
void MouseInteractable<T, Rect, Point>::OnMouseOut()
{
}

template <typename T, typename Rect, typename Point>
void MouseInteractable<T, Rect, Point>::OnMouseIn()
{
}

template <typename T, typename Rect, typename Point>
void MouseInteractable<T, Rect, Point>::OnMouseMove(const Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::OnLMBDown(const Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return ClickEvent::NONE;
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::OnRMBDown(const Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return ClickEvent::NONE;
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::OnLMBUp(const Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return ClickEvent::NONE;
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::OnRMBUp(const Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return ClickEvent::NONE;
}

}