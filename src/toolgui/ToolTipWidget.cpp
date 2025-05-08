#include "ToolTipWidget.hpp"
#include "SDL_Framework/SDLRenderer.hpp"

std::unique_ptr<node::ToolTipWidget> node::ToolTipWidget::Create(TTF_Font* font, std::string text, Widget* parent)
{
    assert(font);
    int measured_width = 0;
    size_t measured_length = 0;
    if (!TTF_MeasureString(font, text.c_str(), text.size(), 0, &measured_width, &measured_length))
    {
        return nullptr;
    }

    int height = TTF_GetFontHeight(font);

    constexpr float side_margin = 4;
    return std::make_unique<ToolTipWidget>(font, std::move(text), 
        WidgetSize{static_cast<float>(measured_width + 2 * side_margin), static_cast<float>(height + 2 * side_margin)}, parent);
}

node::ToolTipWidget::ToolTipWidget(TTF_Font* font, std::string text,
    const WidgetSize& size, Widget* parent)
    : Widget{ size, parent }, m_painter{ font }
{
    m_painter.SetText(std::move(text));
}

void node::ToolTipWidget::OnDraw(SDL::Renderer& renderer)
{
    const SDL_Color text_color = renderer.GetColor(ColorRole::text_normal);

    auto current_rect = GetSize().ToRect();

    const SDL_Color outline_color = renderer.GetColor(ColorRole::frame_outline);
    SDL_SetRenderDrawColor(renderer, outline_color.r, outline_color.g, outline_color.b, outline_color.a);
    SDL_RenderFillRect(renderer, &current_rect);
    current_rect.x += 2;
    current_rect.y += 2;
    current_rect.w -= 4;
    current_rect.h -= 4;

    const SDL_Color background_color = renderer.GetColor(ColorRole::frame_background);
    SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
    SDL_RenderFillRect(renderer, &current_rect);

    m_painter.Draw(renderer, { current_rect.x + 2, current_rect.y + 2 }, text_color);
}