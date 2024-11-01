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
    SDL_FPoint SDLpoint{ static_cast<float>(point.x), static_cast<float>(point.y) };
    SDL_FRect SDLrect{ static_cast<float>(m_rect.x), static_cast<float>(m_rect.y), static_cast<float>(m_rect.w), static_cast<float>(m_rect.h) };
    if (SDL_PointInRectFloat(&SDLpoint, &SDLrect))
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
void MouseInteractable<T, Rect, Point>::MouseMove(MouseHoverEvent& e)
{
    this->OnMouseMove(e);
}

template <typename T, typename Rect, typename Point>
void MouseInteractable<T, Rect, Point>::MouseMove(MouseHoverEvent&& e)
{
    this->OnMouseMove(e);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::LMBDown(MouseButtonEvent& e)
{
    return this->OnLMBDown(e);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::LMBDown(MouseButtonEvent&& e)
{
    return this->OnLMBDown(e);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::RMBDown(MouseButtonEvent& e)
{
    return this->OnRMBDown(e);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::RMBDown(MouseButtonEvent&& e)
{
    return this->OnRMBDown(e);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::RMBUp(MouseButtonEvent& e)
{
    return this->OnRMBUp(e);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::RMBUp(MouseButtonEvent&& e)
{
    return this->OnRMBUp(e);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::LMBUp(MouseButtonEvent& e)
{
    return this->OnLMBUp(e);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::LMBUp(MouseButtonEvent&& e)
{
    return this->OnLMBUp(e);
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
void MouseInteractable<T, Rect, Point>::OnMouseMove(MouseHoverEvent& e)
{
    UNUSED_PARAM(e);
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::OnLMBDown(MouseButtonEvent& e)
{
    UNUSED_PARAM(e);
    return ClickEvent::NONE;
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::OnRMBDown(MouseButtonEvent& e)
{
    UNUSED_PARAM(e);
    return ClickEvent::NONE;
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::OnLMBUp(MouseButtonEvent& e)
{
    UNUSED_PARAM(e);
    return ClickEvent::NONE;
}

template <typename T, typename Rect, typename Point>
ClickEvent MouseInteractable<T, Rect, Point>::OnRMBUp(MouseButtonEvent& e)
{
    UNUSED_PARAM(e);
    return ClickEvent::NONE;
}

}