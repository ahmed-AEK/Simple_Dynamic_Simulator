#pragma once

#include "toolgui/toolgui_exports.h"
#include "toolgui/NodeMacros.h"

#include "toolgui/Handle.hpp"

#include <memory_resource>
#include <SDL_Framework/SDL_headers.h>
#pragma once

#include <memory>

namespace MI{

enum class ClickEvent: int
{
    NONE,
    CAPTURE_START,
    CAPTURE_END,
    CLICKED
};

template <typename T, typename Rect, typename Point>
class TOOLGUI_API MouseInteractable
{
public:
    T* GetInteractableAtPoint(const Point& point);
    void MouseOut(); 
    void MouseIn();
    void MouseMove(const Point& current_mouse_point);
    ClickEvent LMBDown(const Point& current_mouse_point);
    ClickEvent LMBUp(const Point& current_mouse_point);
    ClickEvent RMBDown(const Point& current_mouse_point);
    ClickEvent RMBUp(const Point& current_mouse_point);
    node::HandlePtr<T> GetMIHandlePtr();
    MouseInteractable() = default;
    MouseInteractable(const Rect& rect) : m_rect{ rect } {}

    MouseInteractable(MouseInteractable&& other) noexcept;
    MouseInteractable& operator=(MouseInteractable<T, Rect, Point>&& other) noexcept;
    const Rect& GetRectImpl() const noexcept { return m_rect; }
    void SetRectImpl(const Rect& rect) noexcept { m_rect = rect; }
protected:
    virtual T* OnGetInteractableAtPoint(const Point& point) = 0;
    virtual void OnMouseOut();
    virtual void OnMouseIn();
    virtual void OnMouseMove(const Point& current_mouse_point);
    virtual ClickEvent OnLMBDown(const Point& current_mouse_point);
    virtual ClickEvent OnRMBDown(const Point& current_mouse_point);
    virtual ClickEvent OnLMBUp(const Point& current_mouse_point);
    virtual ClickEvent OnRMBUp(const Point& current_mouse_point);
private:
    Rect m_rect{0,0,0,0};
    node::HandleOwnigPtr<T> MI_handle_ptr = node::HandleAllocator<T>::CreateHandle(static_cast<T*>(this));
};

template <typename T, typename Rect, typename Point>
MouseInteractable<T, Rect, Point>::MouseInteractable(MouseInteractable<T, Rect, Point>&& other) noexcept
    : m_rect(other.m_rect), MI_handle_ptr(std::move(other.MI_handle_ptr.m_ptr))
{
    this->MI_handle_ptr.m_ptr.ptr->object = this;
}
    
template <typename T, typename Rect, typename Point>
MouseInteractable<T, Rect, Point>& MouseInteractable<T, Rect, Point>::operator=(MouseInteractable<T, Rect, Point>&& other) noexcept
{
    this->m_rect = other.m_rect;
    this->MI_handle_ptr.m_ptr = std::move(other.MI_handle_ptr.m_ptr);
    this->MI_handle_ptr.m_ptr.ptr->object = this;
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