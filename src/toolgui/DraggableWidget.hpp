#pragma once

#include "toolgui/toolgui_exports.h"

#include "toolgui/Widget.hpp"
#include <memory>

namespace node
{
    class TOOLGUI_API DragableWidget: public Widget
    {
    public:
        DragableWidget(const WidgetSize& size, node::Widget* parent);
    protected:
        virtual void OnMouseMove(MouseHoverEvent& e) override;
        virtual MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
        virtual MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
        SDL_FPoint m_dragStartPoint{ 0,0 };
        bool b_dragging = false;

    }; 
}