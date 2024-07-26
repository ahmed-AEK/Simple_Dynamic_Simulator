#pragma once

#include "SDL2/SDL.h"
#include "toolgui/toolgui_exports.h"
#include "toolgui/NodeMacros.h"
#include "toolgui/MouseInteractable.hpp"
#include "toolgui/DragDropObject.hpp"

namespace node
{
    class Scene;
    enum class WidgetScaling
    {
        ScaleWithWindow,
        FixedPixels
    };

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
        WidgetScaling GetScalingType() const;
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
    protected:
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
        virtual Widget* OnGetInteractableAtPoint(const SDL_Point& point) override;
        node::Scene* p_parent;
        SDL_Rect m_rect_base;
        WidgetScaling m_scalingType = WidgetScaling::ScaleWithWindow;
    private:
        bool m_isDropTarget = false;
    };
}