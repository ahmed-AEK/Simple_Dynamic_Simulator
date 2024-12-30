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

    template <typename T>
    struct ButtonPressEvent;

    template <typename T>
    struct MouseHoverEvent;

    template <typename T>
    struct MouseButtonEvent;

    template <typename T, typename Rect, typename Point>
    class TOOLGUI_API MouseInteractable
    {
    public:
        using MouseHoverEvent = MI::MouseHoverEvent<T>;
        using MouseButtonEvent = MI::MouseButtonEvent<T>;

        T* GetInteractableAtPoint(const Point& point);

        void MouseOut();
        void MouseIn();
        void MouseMove(MouseHoverEvent& e);
        void MouseMove(MouseHoverEvent&& e);

        ClickEvent LMBDown(MouseButtonEvent& e);
        ClickEvent LMBUp(MouseButtonEvent& e);
        ClickEvent RMBDown(MouseButtonEvent& e);
        ClickEvent RMBUp(MouseButtonEvent& e);

        ClickEvent LMBDown(MouseButtonEvent&& e);
        ClickEvent LMBUp(MouseButtonEvent&& e);
        ClickEvent RMBDown(MouseButtonEvent&& e);
        ClickEvent RMBUp(MouseButtonEvent&& e);

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
        virtual void OnMouseMove(MouseHoverEvent& e);
        
        virtual ClickEvent OnLMBDown(MouseButtonEvent& e);
        virtual ClickEvent OnLMBUp(MouseButtonEvent& e);
        virtual ClickEvent OnRMBDown(MouseButtonEvent& e);
        virtual ClickEvent OnRMBUp(MouseButtonEvent& e);
    private:
        Rect m_rect{ 0,0,0,0 };
        node::HandleOwnigPtr<T> MI_handle_ptr = node::HandleAllocator<T>::CreateHandle(static_cast<T*>(this));
    };
}