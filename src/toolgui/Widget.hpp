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

extern template class MI::MouseInteractable<node::Widget, SDL_Rect, SDL_Point>;

namespace node
{
    class Scene;

    class Widget;
    using WidgetMouseInteractable = MI::MouseInteractable<Widget, SDL_Rect, SDL_Point>;
    class TOOLGUI_API Widget : public WidgetMouseInteractable
    {
    public:
        Widget(const SDL_Rect& rect, Scene* parent);
        void SetRect(const SDL_Rect& rect);
        const SDL_Rect& GetRect() const noexcept { return WidgetMouseInteractable::GetRectImpl(); }
        virtual void Draw(SDL_Renderer* renderer) = 0;
        virtual ~Widget();
        const SDL_Rect& GetBaseRect() noexcept;
        void SetBaseRect(const SDL_Rect& rect) noexcept;
        void InvalidateRect();
        node::Scene* GetScene() const noexcept;
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

        void CharPress(int32_t key) { OnChar(key); }
        void KeyPress(int32_t key) { OnKeyPress(key); }
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
    protected:
        virtual void OnChar(int32_t key) { UNUSED_PARAM(key); } // a or A, etc..
        virtual void OnKeyPress(int32_t key) { UNUSED_PARAM(key); } // SDLK_BACKSPACE and SDLK_RETURN, left and right
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
        node::Scene* p_parent;
        SDL_Rect m_rect_base;
        bool m_isDropTarget = false;
        bool b_focusable = false;
        bool b_focused = false;
    };
}