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

template <typename T>
class TOOLGUI_API MouseInteractable
{
public:
    T* GetInteractableAtPoint(const SDL_Point& point);
    void MouseOut(); 
    void MouseIn();
    void MouseMove(const SDL_Point& current_mouse_point);
    ClickEvent LMBDown(const SDL_Point& current_mouse_point);
    ClickEvent LMBUp(const SDL_Point& current_mouse_point);
    ClickEvent RMBDown(const SDL_Point& current_mouse_point);
    ClickEvent RMBUp(const SDL_Point& current_mouse_point);
    node::HandlePtr<T> GetMIHandlePtr();
    MouseInteractable() = default;
    MouseInteractable(MouseInteractable&& other) noexcept;
    MouseInteractable& operator=(MouseInteractable<T>&& other) noexcept;
    const SDL_Rect& GetRectImpl() const noexcept { return m_rect; }
    void SetRectImpl(const SDL_Rect& rect) noexcept { m_rect = rect; }
protected:
    virtual T* OnGetInteractableAtPoint(const SDL_Point& point) = 0;
    virtual void OnMouseOut();
    virtual void OnMouseIn();
    virtual void OnMouseMove(const SDL_Point& current_mouse_point);
    virtual ClickEvent OnLMBDown(const SDL_Point& current_mouse_point);
    virtual ClickEvent OnRMBDown(const SDL_Point& current_mouse_point);
    virtual ClickEvent OnLMBUp(const SDL_Point& current_mouse_point);
    virtual ClickEvent OnRMBUp(const SDL_Point& current_mouse_point);
private:
    SDL_Rect m_rect{0,0,0,0};
    node::HandleOwnigPtr<T> MI_handle_ptr = node::HandleAllocator<T>::CreateHandle(static_cast<T*>(this));
};

template<typename T>
MouseInteractable<T>::MouseInteractable(MouseInteractable<T>&& other) noexcept
    : m_rect(other.m_rect), MI_handle_ptr(std::move(other.MI_handle_ptr.m_ptr))
{
    this->MI_handle_ptr.m_ptr.ptr->object = this;
}
    
template<typename T>
MouseInteractable<T>& MouseInteractable<T>::operator=(MouseInteractable<T>&& other) noexcept
{
    this->m_rect = other.m_rect;
    this->MI_handle_ptr.m_ptr = std::move(other.MI_handle_ptr.m_ptr);
    this->MI_handle_ptr.m_ptr.ptr->object = this;
	return *this;
}

template <typename T>
T* MouseInteractable<T>::GetInteractableAtPoint(const SDL_Point& point)
{
    if (SDL_PointInRect(&point, &m_rect))
    {
        return this->OnGetInteractableAtPoint(point);
    }
    else
    {
        return nullptr;
    }
}

template <typename T>
void MouseInteractable<T>::MouseOut()
{
    this->OnMouseOut();
}

template <typename T>
void MouseInteractable<T>::MouseIn()
{
    this->OnMouseIn();
}

template <typename T>
void MouseInteractable<T>::MouseMove(const SDL_Point& current_mouse_point)
{
    this->OnMouseMove(current_mouse_point);
}

template <typename T>
ClickEvent MouseInteractable<T>::LMBDown(const SDL_Point& current_mouse_point)
{
    return this->OnLMBDown(current_mouse_point);
}

template <typename T>
ClickEvent MouseInteractable<T>::RMBDown(const SDL_Point& current_mouse_point)
{
    return this->OnRMBDown(current_mouse_point);
}

template <typename T>
ClickEvent MouseInteractable<T>::RMBUp(const SDL_Point& current_mouse_point)
{
    return this->OnRMBUp(current_mouse_point);
}

template <typename T>
ClickEvent MouseInteractable<T>::LMBUp(const SDL_Point& current_mouse_point)
{
    return this->OnLMBUp(current_mouse_point);
}

template <typename T>
node::HandlePtr<T> MouseInteractable<T>::GetMIHandlePtr()
{
    return this->MI_handle_ptr.GetHandlePtr();
}

template <typename T>
void MouseInteractable<T>::OnMouseOut()
{
}

template <typename T>
void MouseInteractable<T>::OnMouseIn()
{
}

template <typename T>
void MouseInteractable<T>::OnMouseMove(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
}

template <typename T>
ClickEvent MouseInteractable<T>::OnLMBDown(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return ClickEvent::NONE;
}

template <typename T>
ClickEvent MouseInteractable<T>::OnRMBDown(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return ClickEvent::NONE;
}

template <typename T>
ClickEvent MouseInteractable<T>::OnLMBUp(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return ClickEvent::NONE;
}

template <typename T>
ClickEvent MouseInteractable<T>::OnRMBUp(const SDL_Point& current_mouse_point)
{
    UNUSED_PARAM(current_mouse_point);
    return ClickEvent::NONE;
}

}