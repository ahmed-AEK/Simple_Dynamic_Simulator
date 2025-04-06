#include "ButtonWidget.hpp"
#include "toolgui/NodeMacros.h"
#include "toolgui/Scene.hpp"
#include "toolgui/ContextMenu.hpp"
#include "toolgui/Application.hpp"
#include "toolgui/ToolTipWidgetMixin_impl.hpp"

template class node::mixin::TooltipMixin<node::ButtonWidget>;

node::ButtonWidget::ButtonWidget(const WidgetSize& size, 
    std::string label, std::string svg_path, std::function<void(void)> action, node::Widget* parent)
: Widget(size, parent), m_btn_painter{svg_path, static_cast<int>(size.w - 9), static_cast<int>(size.h - 9)}, m_action(std::move(action))
{
    SetToolTipDescription(std::move(label));
}

void node::ButtonWidget::OnDraw(SDL::Renderer& renderer)
{
    SDL_FRect btn_rect = GetSize().ToRect();
    SDL_Color bg_color{ 255,255,255,255 };

    if (m_hovered)
    {
        bg_color = { 245, 245, 245, 255 };
    }

    ThickFilledRoundRect(renderer, btn_rect, 8, 2, SDL_Color{ 204,204,204,255 }, bg_color, m_outer_painter, m_inner_painter);

    btn_rect.x += 4;
    btn_rect.y += 4;
    btn_rect.w -= 4;
    btn_rect.h -= 4;

    m_btn_painter.Draw(renderer, btn_rect.x, btn_rect.y);
}

void node::ButtonWidget::OnMouseOut(MouseHoverEvent&)
{
    m_mouse_held = false;
    m_hovered = false;
    ToolTipMouseOut();
}

void node::ButtonWidget::OnMouseIn(MouseHoverEvent& e)
{
    m_hovered = true;
    ToolTipMouseIn(e);
}

void node::ButtonWidget::OnMouseMove(MouseHoverEvent& e)
{
    ToolTipMouseMove(e);
}

MI::ClickEvent node::ButtonWidget::OnLMBDown(MouseButtonEvent& e)
{
    UNUSED_PARAM(e);
    m_mouse_held = true;
    return MI::ClickEvent::CAPTURE_START;
}
MI::ClickEvent node::ButtonWidget::OnLMBUp(MouseButtonEvent& e)
{
    if (m_mouse_held)
    {
        m_logger.LogDebug("Button Pressed!");
        Widget::OnLMBUp(e);
        m_action();
    }
    ToolTipMouseOut();
    return MI::ClickEvent::CAPTURE_END;
}

