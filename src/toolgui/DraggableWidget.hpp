#pragma once

#include "toolgui/toolgui_exports.h"

#include "toolgui/Widget.hpp"
#include <memory>

namespace node
{
    class TOOLGUI_API DragableWidget: public Widget
    {
    public:
        DragableWidget(const SDL_Rect& sceneRect, node::Scene* parent);
    protected:
        virtual void OnMouseMove(const SDL_Point& current_mouse_point) override;
        virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override;
        virtual MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;
        SDL_Point m_dragStartPoint{ 0,0 };
        bool b_dragging = false;

    }; 
}