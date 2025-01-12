#pragma once

#include "toolgui/toolgui_exports.h"
#include "toolgui/NodeMacros.h"

#include "toolgui/Handle.hpp"

#include <memory_resource>
#include <SDL_Framework/SDL_headers.h>

#include <memory>

template <typename T>
concept Sized = requires (const T & obj)
{
    obj.GetSize();
    obj.GetSize().w;
    obj.GetSize().h;
};

template <typename T>
concept Positioned = requires (const T & obj)
{
    obj.GetPosition();
    obj.GetPosition().x;
    obj.GetPosition().y;
};


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

    template <typename T, typename Size, typename Point>
    class TOOLGUI_API MouseInteractable
    {
    public:

        using MI_Type = T;

        using MouseHoverEvent = MI::MouseHoverEvent<T>;
        using MouseButtonEvent = MI::MouseButtonEvent<T>;

        T* GetInteractableAtPoint(const Point& point);

        void MouseOut(MouseHoverEvent& e);
        void MouseIn(MouseHoverEvent& e);
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
        MouseInteractable(const MouseInteractable&) = delete;
        MouseInteractable& operator=(const MouseInteractable&) = delete;
        ~MouseInteractable() = default;

        MouseInteractable(MouseInteractable&& other) noexcept;
        MouseInteractable& operator=(MouseInteractable<T, Size, Point>&& other) noexcept;

    protected:
        virtual T* OnGetInteractableAtPoint(const Point& point);

        virtual void OnMouseOut(MouseHoverEvent& e);
        virtual void OnMouseIn(MouseHoverEvent& e);
        virtual void OnMouseMove(MouseHoverEvent& e);
        
        virtual ClickEvent OnLMBDown(MouseButtonEvent& e);
        virtual ClickEvent OnLMBUp(MouseButtonEvent& e);
        virtual ClickEvent OnRMBDown(MouseButtonEvent& e);
        virtual ClickEvent OnRMBUp(MouseButtonEvent& e);
    private:
        T& Self() requires Sized<T> && Positioned<T> { return *static_cast<T*>(this); }
        const T& Self() const requires Sized<T>&& Positioned<T> { return *static_cast<const T*>(this); }
        node::HandleOwnigPtr<T> MI_handle_ptr = node::HandleAllocator<T>::CreateHandle(static_cast<T*>(this));
    };
}