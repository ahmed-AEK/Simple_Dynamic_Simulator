#pragma once

#include "SDL2/SDL.h"
#include "toolgui/toolgui_exports.h"
#include "toolgui/NodeMacros.h"
#include "toolgui/MouseInteractable.hpp"

namespace node
{
    class Scene;
    enum class WidgetScaling
    {
        ScaleWithWindow,
        FixedPixels
    };

    class TOOLGUI_API Widget : public MI::MouseInteractable<Widget>
    {
    public:
        Widget(SDL_Rect rect, Scene* parent);
        void SetRect(const SDL_Rect& rect);
        virtual void Draw(SDL_Renderer* renderer) = 0;
        virtual ~Widget() noexcept {};
        const SDL_Rect& GetBaseRect() noexcept;
        void InvalidateRect();
        WidgetScaling GetScalingType() const;
        node::Scene* GetScene() const noexcept;
        bool Scroll(const double amount, const SDL_Point& p) {return OnScroll(amount, p);}
    protected:
        virtual void OnSetRect(const SDL_Rect& rect);
        virtual bool OnScroll(const double amount, const SDL_Point& p);
        virtual Widget* OnGetInteractableAtPoint(const SDL_Point& point) override;
        node::Scene* p_parent;
        SDL_Rect m_rect_base;
        WidgetScaling m_scalingType = WidgetScaling::ScaleWithWindow;
    };
}