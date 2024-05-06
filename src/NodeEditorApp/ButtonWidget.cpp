#include "ButtonWidget.hpp"
#include "toolgui/NodeMacros.h"
#include "ExampleContextMenu.hpp"
#include "toolgui/Scene.hpp"

node::ButtonWidget::ButtonWidget(int x, int y, int width, int height, std::string label, std::function<void(void)> action, node::Scene* parent)
: Widget({x,y, width, height}, parent), m_label(std::move(label)), m_action(std::move(action))
{
    m_scalingType = WidgetScaling::ScaleWithWindow;
}
void node::ButtonWidget::Draw(SDL_Renderer* renderer)
{
    if (!m_textSurface)
    {
        SDL_Color Black = { 50, 50, 50, 255 };
        m_textSurface = SDLSurface{ TTF_RenderText_Solid(p_parent->GetApp()->getFont().get(), m_label.c_str(), Black)};
        m_textTexture = SDLTexture{ SDL_CreateTextureFromSurface(renderer, m_textSurface.get()) };
    }
    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_RenderFillRect(renderer, &this->GetRect());
    SDL_Rect inner_rect = GetRect();
    inner_rect.x += w_margin;
    inner_rect.y += h_margin;
    inner_rect.w -= 2 * w_margin;
    inner_rect.h -= 2 * h_margin;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &inner_rect);
    SDL_Rect text_rect{};
    SDL_QueryTexture(m_textTexture.get(), NULL, NULL, &text_rect.w, &text_rect.h);
    text_rect.x = inner_rect.x + inner_rect.w / 2 - text_rect.w / 2;
    text_rect.y = inner_rect.y + inner_rect.h / 2 - text_rect.h / 2;
    SDL_RenderCopy(renderer, m_textTexture.get(), NULL, &text_rect);
}

MI::ClickEvent node::ButtonWidget::OnLMBUp(const SDL_Point& p)
{
    SDL_Log("Button Pressed!");
    Widget::OnLMBUp(p);
    m_action();
    return MI::ClickEvent::CLICKED;
}

