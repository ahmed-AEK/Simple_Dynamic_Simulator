#include "SDL_Framework/Utility.hpp"

#include <mutex>
#include <unordered_map>
#include <array>

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



void DrawFilledArcAA3(SDL_Renderer* renderer, SDL_Point midpoint, int radius, const SDL_Color color);

class RoundRectPainter;
std::mutex painters_mutex{};
std::unordered_map<SDL_Texture*, RoundRectPainter*> painters{};

void textures::ResetAllTextures()
{
    std::lock_guard g{ painters_mutex };
    for (auto&& painter_it : painters)
    {
        painter_it.second->DropTextureNoLock();
    }
    painters.clear();
}

RoundRectPainter::RoundRectPainter(RoundRectPainter&& other) noexcept
    :stored_arc_texture{ std::exchange(other.stored_arc_texture, nullptr) },
    stored_color{ other.stored_color }, stored_radius{ other.stored_radius }
{
    {
        std::lock_guard g{ painters_mutex };
        auto it = painters.find(stored_arc_texture);
        it->second = this;
    }
}

RoundRectPainter& RoundRectPainter::operator=(RoundRectPainter&& other) noexcept
{
    stored_arc_texture = std::exchange(other.stored_arc_texture, nullptr);
    {
        std::lock_guard g{ painters_mutex };
        auto it = painters.find(stored_arc_texture);
        it->second = this;
    }
    stored_color = other.stored_color;
    stored_radius = other.stored_radius;
    return *this;
}

RoundRectPainter::~RoundRectPainter()
{
    if (stored_arc_texture)
    {
        SDL_DestroyTexture(stored_arc_texture);
        {
            std::lock_guard g{ painters_mutex };
            painters.erase(stored_arc_texture);
        }
    }
}

void RoundRectPainter::Draw(SDL_Renderer* renderer, const SDL_Rect rect, int radius, const SDL_Color& color)
{
    if (!stored_arc_texture || radius != stored_radius || color.r != stored_color.r || color.g != stored_color.g || color.b != stored_color.b)
    {
        stored_color = color;
        stored_radius = radius;
        if (stored_arc_texture)
        {
            SDL_DestroyTexture(stored_arc_texture);
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

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    {
        // draw lower right corner
        SDL_Rect trgt{ rect.x + rect.w - stored_radius, rect.y + rect.h - radius, stored_radius, stored_radius };
        SDL_RenderCopyEx(renderer, stored_arc_texture, 0, &trgt, 0, 0, SDL_FLIP_NONE);
    }
    {
        // draw upper right corner
        SDL_Rect trgt{ rect.x + rect.w - stored_radius, rect.y, stored_radius, stored_radius };
        SDL_RenderCopyEx(renderer, stored_arc_texture, 0, &trgt, 270, 0, SDL_FLIP_NONE);
    }
    {
        // draw upper left corner
        SDL_Rect trgt{ rect.x, rect.y, stored_radius, stored_radius };
        SDL_RenderCopyEx(renderer, stored_arc_texture, 0, &trgt, 180, 0, SDL_FLIP_NONE);
    }
    {
        // draw lower left corner
        SDL_Rect trgt{ rect.x, rect.y + rect.h - stored_radius, stored_radius, stored_radius };
        SDL_RenderCopyEx(renderer, stored_arc_texture, 0, &trgt, 90, 0, SDL_FLIP_NONE);
    }
}

void RoundRectPainter::DropTexture()
{
    if (stored_arc_texture)
    {
        {
            std::lock_guard g{ painters_mutex };
            painters.erase(stored_arc_texture);
        }
    }
    stored_arc_texture = nullptr;
}

void RoundRectPainter::DropTextureNoLock()
{
    stored_arc_texture = nullptr;
}

void RoundRectPainter::ReCreateArcTexture(SDL_Renderer* renderer)
{
    SDL_Texture* old_texture = SDL_GetRenderTarget(renderer);
    stored_arc_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, stored_radius, stored_radius);
    SDL_SetTextureBlendMode(stored_arc_texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, stored_arc_texture);
    DrawFilledArcAA3(renderer, { 0,0 }, stored_radius - 1, stored_color);
    SDL_SetRenderTarget(renderer, old_texture);
    {
        std::lock_guard g{ painters_mutex };
        painters.emplace(stored_arc_texture, this);
    }
}
void DrawFilledArcAA3(SDL_Renderer* renderer, SDL_Point midpoint, int radius, const SDL_Color color)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    int y = 0;
    int max_y = radius + 1;
    int raidus_squared = radius * radius;
    while (y <= max_y)
    {
        double x = sqrt(raidus_squared - y * y);
        {
            int max_x = static_cast<int>(floor(x));
            if (floor(x) < y)
            {
                break;
            }
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            std::array<SDL_Point, 4> points{
            SDL_Point{midpoint.x, midpoint.y + y}, SDL_Point{midpoint.x + max_x, midpoint.y + y},
            SDL_Point{midpoint.x + y, midpoint.y}, SDL_Point{midpoint.x + y, midpoint.y + max_x},
            };
            SDL_RenderDrawLines(renderer, points.data(), static_cast<int>(points.size()));
        }

        {
            double overflow = x - floor(x);
            int color_value = static_cast<int>(255 * overflow);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, static_cast<Uint8>(color_value));
            int max_x = static_cast<int>(floor(x + 1));
            std::array<SDL_Point, 2> points{
                SDL_Point{midpoint.x + max_x, midpoint.y + y},
                SDL_Point{midpoint.x + y, midpoint.y + max_x},
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
    int max_y = radius + 1;
    int raidus_squared = radius * radius;
    while (y <= max_y)
    {
        double x = sqrt(raidus_squared - y * y);
        {
            int max_x = static_cast<int>(floor(x));
            if (floor(x) < y)
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
