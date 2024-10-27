#pragma once

#include "SDL2/SDL.h"
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
        SDL_Point point()
        {
            return { e.x,e.y };
        }
    };

    template<>
    struct MouseButtonEvent<node::Widget>
    {
        const SDL_MouseButtonEvent& e;
        SDL_Point point()
        {
            return { e.x,e.y };
        }
    };
}

extern template class MI::MouseInteractable<node::Widget, SDL_Rect, SDL_Point>;

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
    using WidgetMouseInteractable = MI::MouseInteractable<Widget, SDL_Rect, SDL_Point>;
    class TOOLGUI_API Widget : public WidgetMouseInteractable
    {
    public:
        Widget(const SDL_Rect& rect, Widget* parent);
        void SetRect(const SDL_Rect& rect);
        const SDL_Rect& GetRect() const noexcept { return WidgetMouseInteractable::GetRectImpl(); }
        virtual void Draw(SDL_Renderer* renderer) = 0;
        virtual ~Widget();
        const SDL_Rect& GetBaseRect() noexcept;
        void SetBaseRect(const SDL_Rect& rect) noexcept;
        bool Scroll(const double amount, const SDL_Point& p) {return OnScroll(amount, p);}
        
        bool IsDropTarget() const { return m_isDropTarget; }
        void DropEnter(const DragDropObject& object) { OnDropEnter(object); }
        void DropExit(const DragDropObject& object) { OnDropExit(object); }
        void DropHover(const DragDropObject& object, const SDL_Point& p) 
        { 
            OnDropHover(object, p);
        }
        void DropObject(DragDropObject& object, const SDL_Point& p)
        {
            OnDropObject(object, p);
        }
        void DrawDropObject(SDL_Renderer* renderer, 
            const DragDropObject& object, const SDL_Point& p)
        {
            OnDrawDropObject(renderer, object, p);
        }

        void CharPress(TextInputEvent& e) { OnChar(e); }
        void CharPress(TextInputEvent&& e) { OnChar(e); }
        void KeyPress(KeyboardEvent& e) { OnKeyPress(e); }
        void KeyPress(KeyboardEvent&& e) { OnKeyPress(e); }
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
    protected:
        virtual void OnChar(TextInputEvent& e) { UNUSED_PARAM(e); } // a or A, etc..
        virtual void OnKeyPress(KeyboardEvent& e) { UNUSED_PARAM(e); } // SDLK_BACKSPACE and SDLK_RETURN, left and right
        virtual void OnKeyboardFocusIn() {}
        virtual void OnKeyboardFocusOut() {}

        virtual void OnDropEnter(const DragDropObject& object) { UNUSED_PARAM(object); };
        virtual void OnDropExit(const DragDropObject& object) { UNUSED_PARAM(object); };
        virtual void OnDropHover(const DragDropObject& object, const SDL_Point& p) 
        { 
            UNUSED_PARAM(object);
            UNUSED_PARAM(p);
        };
        virtual void OnDropObject(DragDropObject& object, const SDL_Point& p) 
        {
            UNUSED_PARAM(object);
            UNUSED_PARAM(p);
        };
        virtual void OnDrawDropObject(SDL_Renderer* renderer, 
            const DragDropObject& object, const SDL_Point& p)
        {
            object.Draw(renderer, p);
        }
        void SetDropTarget(bool value = true) { m_isDropTarget = value; }
        virtual void OnSetRect(const SDL_Rect& rect);
        virtual bool OnScroll(const double amount, const SDL_Point& p);
        Widget* OnGetInteractableAtPoint(const SDL_Point& point) override;
    private:
        Widget* m_parent;
        SDL_Rect m_rect_base;
        bool m_isDropTarget = false;
        bool b_focusable = false;
        bool b_focused = false;
    };
}