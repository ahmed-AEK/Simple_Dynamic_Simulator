#pragma once

#include "toolgui/toolgui_exports.h"
#include "toolgui/NodeMacros.h"
#include "toolgui/MouseInteractable_interface.hpp"
#include "toolgui/DragDropObject.hpp"


namespace node
{
    class Widget;
}
namespace SDL
{
    class Renderer;
}

namespace MI
{
    template<>
    struct MouseHoverEvent<node::Widget>
    {
        MouseHoverEvent(const SDL_MouseMotionEvent& e)
            :local_position{}, global_pos{e.x, e.y} {}

        MouseHoverEvent(const SDL_FPoint& global_pos)
            :local_position{}, global_pos{ global_pos.x, global_pos.y } {}

        SDL_FPoint local_position;
        SDL_FPoint global_pos;
        SDL_FPoint globalPosition()
        {
            return global_pos;
        }
        SDL_FPoint positon()
        {
            return local_position;
        }
        SDL_FPoint point()
        {
            return local_position;
        }
    };

    template<>
    struct MouseButtonEvent<node::Widget>
    {
        MouseButtonEvent(const SDL_MouseButtonEvent& e)
            : e{ e }, local_position{} {}

        const SDL_MouseButtonEvent& e;
        SDL_FPoint local_position;
        SDL_FPoint globalPosition()
        {
            return { e.x, e.y };
        }
        SDL_FPoint positon()
        {
            return local_position;
        }
        SDL_FPoint point()
        {
            return local_position;
        }
    };
}

extern template class MI::MouseInteractable<node::Widget, SDL_FRect, SDL_FPoint>;

namespace node
{
    class Scene;
    class Application;
    class Widget;

    struct WidgetSize
    {
        float w;
        float h;

        SDL_FRect ToRect() const noexcept
        {
            return SDL_FRect{ 0,0,w,h };
        }

        SDL_FPoint ToFPoint() const noexcept
        {
            return SDL_FPoint{ w,h };
        }
    };

    struct TextInputEvent
    {
        SDL_TextInputEvent e;
    };
    struct KeyboardEvent
    {
        SDL_KeyboardEvent e;
    };
    using WidgetMouseInteractable = MI::MouseInteractable<Widget, WidgetSize, SDL_FPoint>;
    class TOOLGUI_API Widget : public WidgetMouseInteractable
    {
    public:
        Widget(const WidgetSize& size, Widget* parent);
        Widget(const Widget&) = delete;
        Widget& operator=(const Widget&) = delete;

        void SetPosition(const SDL_FPoint& pos);
        void SetSize(const WidgetSize& size);
        virtual void Draw(SDL::Renderer& renderer);
        virtual ~Widget();
        bool Scroll(const double amount, const SDL_FPoint& p) {return OnScroll(amount, p);}
        
        bool IsDropTarget() const { return m_isDropTarget; }
        void DropEnter(const DragDropObject& object) { OnDropEnter(object); }
        void DropExit(const DragDropObject& object) { OnDropExit(object); }
        void DropHover(const DragDropObject& object, const SDL_FPoint& p) 
        { 
            OnDropHover(object, p);
        }
        void DropObject(DragDropObject& object, const SDL_FPoint& p)
        {
            OnDropObject(object, p);
        }
        void DrawDropObject(SDL_Renderer* renderer, 
            const DragDropObject& object, const SDL_FPoint& p)
        {
            OnDrawDropObject(renderer, object, p);
        }

        bool CharPress(TextInputEvent& e) { return OnChar(e); }
        bool CharPress(TextInputEvent&& e) { return OnChar(e); }
        bool KeyPress(KeyboardEvent& e) { return OnKeyPress(e); }
        bool KeyPress(KeyboardEvent&& e) { return OnKeyPress(e); }
        void SetFocusable(bool value = true) { b_focusable = value; }
        bool IsFocusable() const { return b_focusable; }
        void SetFocused(bool value = true) { 
            b_focused = value;
            if (value)
            {
                OnKeyboardFocusIn();
            }
            else
            {
                OnKeyboardFocusOut();
            }
        }
        virtual Widget* GetFocusable();
        virtual Application* GetApp() const;
        Widget* GetParent() const { return m_parent; }
        void SetParent(Widget* parent);
        void UnParent();
        void SetFocusProxy(Widget* other);
        Widget* GetFocusProxy() const { return m_focus_proxy.GetObjectPtr(); }

        SDL_FPoint GetGlobalPosition() const;
        SDL_FPoint GetPosition() const { return m_position; }
        WidgetSize GetSize() const { return m_size; }

        static SDL_Rect WidgetRect(node::Widget& widget)
        {
            auto pos = widget.GetPosition();
            auto size = widget.GetSize();
            return SDL_Rect{
                static_cast<int>(pos.x),
                static_cast<int>(pos.y),
                static_cast<int>(size.w),
                static_cast<int>(size.h) };
        }
    protected:
        virtual bool OnChar(TextInputEvent& e) { UNUSED_PARAM(e); return false; } // a or A, etc..
        virtual bool OnKeyPress(KeyboardEvent& e) { UNUSED_PARAM(e); return false;  } // SDLK_BACKSPACE and SDLK_RETURN, left and right
        virtual void OnKeyboardFocusIn() {}
        virtual void OnKeyboardFocusOut() {}

        virtual void OnDropEnter(const DragDropObject& object) { UNUSED_PARAM(object); };
        virtual void OnDropExit(const DragDropObject& object) { UNUSED_PARAM(object); };
        virtual void OnDropHover(const DragDropObject& object, const SDL_FPoint& p) 
        { 
            UNUSED_PARAM(object);
            UNUSED_PARAM(p);
        };
        virtual void OnDropObject(DragDropObject& object, const SDL_FPoint& p) 
        {
            UNUSED_PARAM(object);
            UNUSED_PARAM(p);
        };
        virtual void OnDrawDropObject(SDL_Renderer* renderer, 
            const DragDropObject& object, const SDL_FPoint& p)
        {
            object.Draw(renderer, p);
        }
        virtual void OnDraw(SDL::Renderer& renderer);
        void SetDropTarget(bool value = true) { m_isDropTarget = value; }
        virtual void OnSetPosition(const SDL_FPoint& pos);
        virtual void OnSetSize(const WidgetSize& size);
        virtual bool OnScroll(const double amount, const SDL_FPoint& p);
        Widget* OnGetInteractableAtPoint(const SDL_FPoint& point) override;
    private:
        Widget* m_parent = nullptr;
        SDL_FPoint m_position{};
        WidgetSize m_size{};
        bool m_isDropTarget = false;
        bool b_focusable = false;
        bool b_focused = false;
        HandlePtr<Widget> m_focus_proxy;
        std::vector<Widget*> m_children;
    };
}