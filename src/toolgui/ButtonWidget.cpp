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

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &btn_rect);
    btn_rect.x += 2;
    btn_rect.y += 2;
    btn_rect.w -= 4;
    btn_rect.h -= 4;

    if (!m_hovered)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    }

    SDL_RenderFillRect(renderer, &btn_rect);
    m_btn_painter.Draw(renderer, btn_rect.x + 2, btn_rect.y + 2);
}

void node::ButtonWidget::OnMouseOut(MouseHoverEvent& e)
{
    m_hovered = false;
    ToolTipMouseOut(e);
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

MI::ClickEvent node::ButtonWidget::OnLMBUp(MouseButtonEvent& e)
{
    SDL_Log("Button Pressed!");
    Widget::OnLMBUp(e);
    m_action();
    return MI::ClickEvent::CLICKED;
}

