#pragma once

#include "toolgui/toolgui_exports.h"
#include "toolgui/NodeMacros.h"

#include "toolgui/Handle.hpp"

#include <memory_resource>
#include <SDL_Framework/SDL_headers.h>

#include <memory>

namespace MI {

    enum class ClickEvent : int
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
        ~MouseInteractable() = default;

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
        virtual ClickEvent OnLMBUp(const Point& current_mouse_point);
        virtual ClickEvent OnRMBDown(const Point& current_mouse_point);
        virtual ClickEvent OnRMBUp(const Point& current_mouse_point);
    private:
        Rect m_rect{ 0,0,0,0 };
        node::HandleOwnigPtr<T> MI_handle_ptr = node::HandleAllocator<T>::CreateHandle(static_cast<T*>(this));
    };
}