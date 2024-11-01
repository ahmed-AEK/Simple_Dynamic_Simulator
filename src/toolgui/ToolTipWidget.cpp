#include "ToolTipWidget.hpp"

node::ToolTipWidget::ToolTipWidget(TTF_Font* font, std::string text, const SDL_FRect& rect, Widget* parent)
    : Widget{ rect,parent }, m_painter{ font }
{
    m_painter.SetText(std::move(text));
}

void node::ToolTipWidget::Draw(SDL_Renderer* renderer)
{
    SDL_Color Black = { 50, 50, 50, 255 };
    SDL_FRect text_rect = m_painter.GetRect(renderer, Black);
    SDL_FRect current_rect = GetRect();
    SDL_FRect desired_rect = text_rect;
    desired_rect.w += 8;
    desired_rect.h += 8;
    if (desired_rect.w != current_rect.w || desired_rect.h != current_rect.h)
    {
        desired_rect.x = current_rect.x;
        desired_rect.y = current_rect.y;
        SetRect(desired_rect);
        current_rect = GetRect();
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &current_rect);
    current_rect.x += 2;
    current_rect.y += 2;
    current_rect.w -= 4;
    current_rect.h -= 4;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &current_rect);

    m_painter.Draw(renderer, { current_rect.x + 2, current_rect.y + 2 }, Black);
}