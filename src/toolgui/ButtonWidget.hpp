#pragma once

#include "toolgui/Widget.hpp"
#include "toolgui/ToolTipWidgetMixin.hpp"
#include "SDL_Framework/SDLCPP.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"

#include <functional>


namespace node
{
    class ButtonWidget;
    extern template class node::mixin::TooltipMixin<node::ButtonWidget>;

    class ButtonWidget: public Widget, public mixin::TooltipMixin<ButtonWidget>
    {
    public:
        ButtonWidget(const WidgetSize& size, std::string label, std::string svg_path, std::function<void(void)> action, node::Widget* parent);
        void OnDraw(SDL::Renderer& renderer) override;
        void SetAction(std::function<void()> action) { m_action = action; }
    protected:
        void OnMouseOut(MouseHoverEvent& e) override;
        void OnMouseIn(MouseHoverEvent& e) override;
        void OnMouseMove(MouseHoverEvent& e) override;
        MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
    private:
        constexpr static int w_margin = 5;
        constexpr static int h_margin = 5;
        SVGRasterizer m_btn_painter;
        RoundRectPainter m_outer_painter;
        RoundRectPainter m_inner_painter;
        std::function<void(void)> m_action;
        bool m_hovered{};
    };
}