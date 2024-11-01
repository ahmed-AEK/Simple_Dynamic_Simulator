#pragma once

#include "toolgui/toolgui_exports.h"
#include "toolgui/NodeMacros.h"
#include "toolgui/MouseInteractable_interface.hpp"
#include "toolgui/DragDropObject.hpp"


namespace node
{
    class Widget;
}

namespace MI
{
    template<>
    struct MouseHoverEvent<node::Widget>
    {
        const SDL_MouseMotionEvent& e;
        SDL_FPoint point()
        {
            return { e.x,e.y };
        }
    };

    template<>
    struct MouseButtonEvent<node::Widget>
    {
        const SDL_MouseButtonEvent& e;
        SDL_FPoint point()
        {
            return { e.x,e.y };
        }
    };
}

extern template class MI::MouseInteractable<node::Widget, SDL_FRect, SDL_FPoint>;

namespace node
{
    class Scene;
    class Application;
    class Widget;

    struct TextInputEvent
    {
        SDL_TextInputEvent e;
    };
    struct KeyboardEvent
    {
        SDL_KeyboardEvent e;
    };
    using WidgetMouseInteractable = MI::MouseInteractable<Widget, SDL_FRect, SDL_FPoint>;
    class TOOLGUI_API Widget : public WidgetMouseInteractable
    {
    public:
        Widget(const SDL_FRect& rect, Widget* parent);
        void SetRect(const SDL_FRect& rect);
        const SDL_FRect& GetRect() const noexcept { return WidgetMouseInteractable::GetRectImpl(); }
        virtual void Draw(SDL_Renderer* renderer) = 0;
        virtual ~Widget();
        const SDL_FRect& GetBaseRect() noexcept;
        void SetBaseRect(const SDL_FRect& rect) noexcept;
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
        void SetParent(Widget* parent) { m_parent = parent; }
        void SetFocusProxy(Widget* other);
        Widget* GetFocusProxy() const { return m_focus_proxy.GetObjectPtr(); }
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
        void SetDropTarget(bool value = true) { m_isDropTarget = value; }
        virtual void OnSetRect(const SDL_FRect& rect);
        virtual bool OnScroll(const double amount, const SDL_FPoint& p);
        Widget* OnGetInteractableAtPoint(const SDL_FPoint& point) override;
    private:
        HandlePtr<Widget> m_focus_proxy;
        Widget* m_parent;
        SDL_FRect m_rect_base;
        bool m_isDropTarget = false;
        bool b_focusable = false;
        bool b_focused = false;
    };
}