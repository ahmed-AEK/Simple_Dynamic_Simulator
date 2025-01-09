#pragma once

#include "toolgui/MouseInteractable_interface.hpp"

namespace MI
{

template <typename T, typename Size, typename Point>
MouseInteractable<T, Size, Point>::MouseInteractable(MouseInteractable<T, Size, Point>&& other) noexcept
    : MI_handle_ptr(std::move(other.MI_handle_ptr.m_ptr))
{
    this->MI_handle_ptr.m_ptr->object = static_cast<T*>(this);
}
    
template <typename T, typename Size, typename Point>
MouseInteractable<T, Size, Point>& MouseInteractable<T, Size, Point>::operator=(MouseInteractable<T, Size, Point>&& other) noexcept
{
    this->MI_handle_ptr.m_ptr = std::move(other.MI_handle_ptr.m_ptr);
    this->MI_handle_ptr.m_ptr->object = static_cast<T*>(this);
	return *this;
}

template <typename T, typename Size, typename Point>
T* MouseInteractable<T, Size, Point>::GetInteractableAtPoint(const Point& point)
{
    const auto& size = Self().GetSize();
    if (point.x >= 0 && point.y >= 0 && size.w > point.x && size.h > point.y)
    {
        return this->OnGetInteractableAtPoint(point);
    }
    return nullptr;
}

template <typename T, typename Size, typename Point>
T* MouseInteractable<T, Size, Point>::OnGetInteractableAtPoint(const Point&)
{
    return &Self();
}

template <typename T, typename Size, typename Point>
void MouseInteractable<T, Size, Point>::MouseOut(MouseHoverEvent& e)
{
    this->OnMouseOut(e);
}

template <typename T, typename Size, typename Point>
void MouseInteractable<T, Size, Point>::MouseIn(MouseHoverEvent& e)
{
    this->OnMouseIn(e);
}

template <typename T, typename Size, typename Point>
void MouseInteractable<T, Size, Point>::MouseMove(MouseHoverEvent& e)
{
    this->OnMouseMove(e);
}

template <typename T, typename Size, typename Point>
void MouseInteractable<T, Size, Point>::MouseMove(MouseHoverEvent&& e)
{
    this->OnMouseMove(e);
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::LMBDown(MouseButtonEvent& e)
{
    return this->OnLMBDown(e);
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::LMBDown(MouseButtonEvent&& e)
{
    return this->OnLMBDown(e);
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::RMBDown(MouseButtonEvent& e)
{
    return this->OnRMBDown(e);
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::RMBDown(MouseButtonEvent&& e)
{
    return this->OnRMBDown(e);
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::RMBUp(MouseButtonEvent& e)
{
    return this->OnRMBUp(e);
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::RMBUp(MouseButtonEvent&& e)
{
    return this->OnRMBUp(e);
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::LMBUp(MouseButtonEvent& e)
{
    return this->OnLMBUp(e);
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::LMBUp(MouseButtonEvent&& e)
{
    return this->OnLMBUp(e);
}

template <typename T, typename Size, typename Point>
node::HandlePtr<T> MouseInteractable<T, Size, Point>::GetMIHandlePtr()
{
    return this->MI_handle_ptr.GetHandlePtr();
}

template <typename T, typename Size, typename Point>
void MouseInteractable<T, Size, Point>::OnMouseOut(MouseHoverEvent& e)
{
    UNUSED_PARAM(e);
}

template <typename T, typename Size, typename Point>
void MouseInteractable<T, Size, Point>::OnMouseIn(MouseHoverEvent& e)
{
    UNUSED_PARAM(e);
}

template <typename T, typename Size, typename Point>
void MouseInteractable<T, Size, Point>::OnMouseMove(MouseHoverEvent& e)
{
    UNUSED_PARAM(e);
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::OnLMBDown(MouseButtonEvent& e)
{
    UNUSED_PARAM(e);
    return ClickEvent::NONE;
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::OnRMBDown(MouseButtonEvent& e)
{
    UNUSED_PARAM(e);
    return ClickEvent::NONE;
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::OnLMBUp(MouseButtonEvent& e)
{
    UNUSED_PARAM(e);
    return ClickEvent::NONE;
}

template <typename T, typename Size, typename Point>
ClickEvent MouseInteractable<T, Size, Point>::OnRMBUp(MouseButtonEvent& e)
{
    UNUSED_PARAM(e);
    return ClickEvent::NONE;
}

}