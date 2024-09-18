#include "SDL_Framework/Utility.hpp"

#include <mutex>
#include <unordered_map>
#include <array>
#include <cassert>
#include <utility>

void FilledRoundRect(SDL_Renderer* renderer, const SDL_Rect& rect, int radius, const SDL_Color& color)
{
    DrawRoundRect(renderer, rect, radius, color);
}

void ThickFilledRoundRect(SDL_Renderer* renderer, const SDL_Rect& original_rect, int original_radius, int thickness, const SDL_Color& color1, const SDL_Color& color2)
{
    {
        SDL_Rect rect = original_rect;
        int radius = original_radius;
        FilledRoundRect(renderer, rect, radius, color1);
    }

    {
        SDL_Rect rect;
        rect.x = original_rect.x + thickness;
        rect.y = original_rect.y + thickness;
        rect.w = original_rect.w - 2 * thickness;
        rect.h = original_rect.h - 2 * thickness;
        int radius = original_radius - thickness;
        FilledRoundRect(renderer, rect, radius, color2);
    }
}



void DrawFilledArcAA3(SDL_Renderer* renderer, int radius, const SDL_Color color);

std::mutex painters_mutex{};

std::unordered_map<SDL_Texture*, DroppableTexture*> Texturepainters{};


void textures::ResetAllTextures()
{
    std::lock_guard g{ painters_mutex };
    for (auto&& painter_it : Texturepainters)
    {
        painter_it.second->DropTextureNoLock();
    }
    Texturepainters.clear();
}



void RoundRectPainter::Draw(SDL_Renderer* renderer, const SDL_Rect rect, int radius, const SDL_Color& color)
{
    if (!m_arc_texture.GetTexture() || radius != stored_radius || color.r != stored_color.r || color.g != stored_color.g || color.b != stored_color.b)
    {
        stored_color = color;
        stored_radius = radius;
        if (m_arc_texture.GetTexture())
        {
            m_arc_texture.DropTexture();
        }
        ReCreateArcTexture(renderer);
    }
    {
        // draw rects
        SDL_SetRenderDrawColor(renderer, stored_color.r, stored_color.g, stored_color.b, 255);
        SDL_Rect rects[]{
            {rect.x + radius, rect.y, rect.w - 2 * radius, radius},
            {rect.x, rect.y + radius, rect.w, rect.h - 2 * radius},
            {rect.x + radius, rect.y + rect.h - radius, rect.w - 2 * radius, radius},
        };

        SDL_RenderFillRects(renderer, rects, static_cast<int>(std::size(rects)));
    }

    {
        // draw lower right corner
        SDL_Rect trgt{ rect.x + rect.w - stored_radius, rect.y + rect.h - radius, stored_radius, stored_radius };
        SDL_RenderCopyEx(renderer, m_arc_texture.GetTexture(), 0, &trgt, 0, 0, SDL_FLIP_NONE);
    }
    {
        // draw upper right corner
        SDL_Rect trgt{ rect.x + rect.w - stored_radius, rect.y, stored_radius, stored_radius };
        SDL_RenderCopyEx(renderer, m_arc_texture.GetTexture(), 0, &trgt, 270, 0, SDL_FLIP_NONE);
    }
    {
        // draw upper left corner
        SDL_Rect trgt{ rect.x, rect.y, stored_radius, stored_radius };
        SDL_RenderCopyEx(renderer, m_arc_texture.GetTexture(), 0, &trgt, 180, 0, SDL_FLIP_NONE);
    }
    {
        // draw lower left corner
        SDL_Rect trgt{ rect.x, rect.y + rect.h - stored_radius, stored_radius, stored_radius };
        SDL_RenderCopyEx(renderer, m_arc_texture.GetTexture(), 0, &trgt, 90, 0, SDL_FLIP_NONE);
    }
}

void RoundRectPainter::ReCreateArcTexture(SDL_Renderer* renderer)
{
    SDL_Texture* old_texture = SDL_GetRenderTarget(renderer);
    SDLTexture stored_arc_texture{ SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, stored_radius, stored_radius) };
    m_arc_texture.SetTexture(std::move(stored_arc_texture));
    SDL_SetTextureBlendMode(m_arc_texture.GetTexture(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, m_arc_texture.GetTexture());
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0); // set color to transparent white
    SDL_RenderClear(renderer); // clear renderer
    DrawFilledArcAA3(renderer, stored_radius - 1, stored_color);
    SDL_SetRenderTarget(renderer, old_texture);
}

void DrawFilledArcAA3(SDL_Renderer* renderer, int radius, const SDL_Color color)
{
    int y = 0;
    int max_y = radius;
    int raidus_squared = radius * radius;
    while (y <= max_y)
    {
        double x = sqrt(raidus_squared - y * y);
        if (std::isnan(x))
        {
            break;
        }
        {
            int max_x = static_cast<int>(floor(x));
            if (floor(x) + 2 < y)
            {
                break;
            }
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            std::array<SDL_Point, 4> points{
            SDL_Point{0, y}, SDL_Point{max_x,y},
            SDL_Point{y, 0}, SDL_Point{y, max_x},
            };
            SDL_RenderDrawLines(renderer, points.data(), static_cast<int>(points.size()));
        }

        {
            double overflow = x - floor(x);
            int color_value = static_cast<int>(255 * overflow);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, static_cast<Uint8>(color_value));
            int max_x = static_cast<int>(floor(x + 1));
            std::array<SDL_Point, 2> points{
                SDL_Point{max_x, y},
                SDL_Point{y, max_x},
            };

            SDL_RenderDrawPoints(renderer, points.data(), static_cast<int>(points.size()));
        }

        y++;
    }
}



enum class ArcDirection
{
    pos = 1,
    neg = -1,
};


template <ArcDirection x_mul, ArcDirection y_mul>
void DrawFilledArcAA4(SDL_Renderer* renderer, SDL_Point midpoint, int radius, const SDL_Color color)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    const int x_mul_i = static_cast<int>(x_mul);
    const int y_mul_i = static_cast<int>(y_mul);
    int y = 0;
    int max_y = radius;
    int raidus_squared = radius * radius;
    while (y <= max_y)
    {
        double x = sqrt(raidus_squared - y * y);
        if (std::isnan(x))
        {
            break;
        }
        {
            int max_x = static_cast<int>(floor(x));
            if (floor(x) + 2 < y)
            {
                break;
            }
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            std::array<SDL_Point, 4> points{
            SDL_Point{midpoint.x, midpoint.y + y * y_mul_i}, SDL_Point{midpoint.x + max_x * x_mul_i, midpoint.y + y * y_mul_i},
            SDL_Point{midpoint.x + y * x_mul_i, midpoint.y}, SDL_Point{midpoint.x + y * x_mul_i, midpoint.y + max_x * y_mul_i},
            };
            SDL_RenderDrawLines(renderer, points.data(), static_cast<int>(points.size()));
        }

        {
            double overflow = x - floor(x);
            int color_value = static_cast<int>(255 * overflow);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, static_cast<Uint8>(color_value));
            int max_x = static_cast<int>(floor(x + 1));
            std::array<SDL_Point, 2> points{
                SDL_Point{midpoint.x + max_x * x_mul_i, midpoint.y + y * y_mul_i},
                SDL_Point{midpoint.x + y * x_mul_i, midpoint.y + max_x * y_mul_i},
            };

            SDL_RenderDrawPoints(renderer, points.data(), static_cast<int>(points.size()));
        }

        y++;
    }
}

void DrawRoundRect(SDL_Renderer* renderer, const SDL_Rect& rect, int radius, const SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

    SDL_Rect rects[]{
        {rect.x + radius, rect.y, rect.w - 2 * radius, radius},
        {rect.x, rect.y + radius, rect.w, rect.h - 2 * radius},
        {rect.x + radius, rect.y + rect.h - radius, rect.w - 2 * radius, radius},
    };

    SDL_RenderFillRects(renderer, rects, static_cast<int>(std::size(rects)));

    DrawFilledArcAA4<ArcDirection::pos, ArcDirection::pos>(renderer, { rect.x + rect.w - radius, rect.y + rect.h - radius }, radius - 1, color);
    DrawFilledArcAA4<ArcDirection::neg, ArcDirection::pos>(renderer, { rect.x + radius - 1, rect.y + rect.h - radius }, radius - 1, color);
    DrawFilledArcAA4<ArcDirection::neg, ArcDirection::neg>(renderer, { rect.x + radius - 1, rect.y + radius - 1 }, radius - 1, color);
    DrawFilledArcAA4<ArcDirection::pos, ArcDirection::neg>(renderer, { rect.x + rect.w - radius, rect.y + radius - 1 }, radius - 1, color);

}

void ThickFilledRoundRect(SDL_Renderer* renderer, const SDL_Rect& original_rect, int original_radius, int thickness, const SDL_Color& color1, const SDL_Color& color2, RoundRectPainter& outer, RoundRectPainter& inner)
{
    {
        SDL_Rect rect = original_rect;
        int radius = original_radius;
        outer.Draw(renderer, rect, radius, color1);
    }

    {
        SDL_Rect rect;
        rect.x = original_rect.x + thickness;
        rect.y = original_rect.y + thickness;
        rect.w = original_rect.w - 2 * thickness;
        rect.h = original_rect.h - 2 * thickness;
        int radius = original_radius - thickness;
        inner.Draw(renderer, rect, radius, color2);
    }
}

void TextPainter::Draw(SDL_Renderer* renderer, const SDL_Point point, const SDL_Color color)
{
    assert(m_font);
    AssureTexture(renderer, color);

    SDL_Rect text_rect{ 0,0,0,0 };
    SDL_QueryTexture(m_text_texture.GetTexture(), NULL, NULL, &text_rect.w, &text_rect.h);
    text_rect.x = point.x;
    text_rect.y = point.y;
    SDL_RenderCopy(renderer, m_text_texture.GetTexture(), NULL, &text_rect);
}

SDL_Rect TextPainter::GetRect(SDL_Renderer* renderer, const SDL_Color color)
{
    assert(m_font);
    AssureTexture(renderer, color);
    SDL_Rect text_rect{ 0,0,0,0 };
    SDL_QueryTexture(m_text_texture.GetTexture(), NULL, NULL, &text_rect.w, &text_rect.h);
    return text_rect;
}

void TextPainter::SetText(std::string text)
{
    m_text = std::move(text);
    m_text_texture.DropTexture();
}

void TextPainter::ReCreateTexture(SDL_Renderer* renderer)
{
    SDL_Color color = m_stored_color;
    auto textSurface = SDLSurface{ TTF_RenderText_Blended(m_font, m_text.c_str(), color) };
    auto texture = SDLTexture{ SDL_CreateTextureFromSurface(renderer, textSurface.get()) };
    m_text_texture.SetTexture(std::move(texture));
    SDL_Rect text_rect{};
    SDL_QueryTexture(m_text_texture.GetTexture(), NULL, NULL, &text_rect.w, &text_rect.h);
}

void TextPainter::AssureTexture(SDL_Renderer* renderer, const SDL_Color& color)
{
    assert(m_font);
    if (!m_text_texture.GetTexture() || color.r != m_stored_color.r || color.g != m_stored_color.g || color.b != m_stored_color.b)
    {
        m_stored_color = color;
        m_text_texture.DropTexture();
        ReCreateTexture(renderer);
    }
}

void TextPainter::SetFont(TTF_Font* font)
{
    m_font = font;
    m_text_texture.DropTexture();
}

DroppableTexture::DroppableTexture(DroppableTexture&& other) noexcept
{
    m_stored_texture = std::move(other.m_stored_texture);
    if (m_stored_texture)
    {
        std::lock_guard g{ painters_mutex };
        auto it = Texturepainters.find(m_stored_texture.get());
        it->second = this;
    }
}

DroppableTexture& DroppableTexture::operator=(DroppableTexture&& other) noexcept
{
    if (&other == this)
    {
        return *this;
    }
    DropTexture();
    m_stored_texture = std::move(other.m_stored_texture);
    if (m_stored_texture)
    {
        std::lock_guard g{ painters_mutex };
        auto it = Texturepainters.find(m_stored_texture.get());
        it->second = this;
    }
    return *this;
}

DroppableTexture::~DroppableTexture()
{
    DropTexture();
}

void DroppableTexture::DropTexture()
{
    if (m_stored_texture)
    {
        std::lock_guard g{ painters_mutex };
        Texturepainters.erase(m_stored_texture.get());   
    }
    m_stored_texture = nullptr;
}

void DroppableTexture::DropTextureNoLock()
{
    m_stored_texture = nullptr;
}

void DroppableTexture::SetTexture(SDLTexture texture)
{
    DropTexture();
    m_stored_texture = std::move(texture);
    if (m_stored_texture)
    {
        std::lock_guard g{ painters_mutex };
        Texturepainters.emplace(m_stored_texture.get(), this);
    }
}

SDL_Texture* DroppableTexture::GetTexture()
{
    return m_stored_texture.get();
}
