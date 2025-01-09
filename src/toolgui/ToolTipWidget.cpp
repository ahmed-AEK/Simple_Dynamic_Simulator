#include "ToolTipWidget.hpp"
#include "SDL_Framework/SDLRenderer.hpp"

node::ToolTipWidget::ToolTipWidget(TTF_Font* font, std::string text, 
    const WidgetSize& size, Widget* parent)
    : Widget{ size, parent }, m_painter{ font }
{
    m_painter.SetText(std::move(text));
}

void node::ToolTipWidget::OnDraw(SDL::Renderer& renderer)
{
    SDL_Color Black = { 50, 50, 50, 255 };
    SDL_FRect text_rect = m_painter.GetRect(renderer, Black);
    SDL_FRect current_rect = GetSize().ToRect();
    WidgetSize desired_size{ text_rect.w, text_rect.h };
    desired_size.w += 8;
    desired_size.h += 8;
    if (desired_size.w != current_rect.w || desired_size.h != current_rect.h)
    {
        SetSize(desired_size);
    }

    current_rect = GetSize().ToRect();
    auto r = renderer.ClipRect(ToRect(current_rect)); // reset clip rect

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