#include "SDL_Framework/Utility.hpp"

#include "sdl2_gfx/SDL2_gfxPrimitives.h"

void FilledRoundRect(SDL_Renderer* renderer, const SDL_Rect& rect, int radius, const SDL_Color& color)
{
    // Set render color to blue ( rect will be rendered in this color )
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    {
        SDL_Rect inner_rect{ rect.x, rect.y + radius, rect.w, rect.h - 2 * radius };

        // Render rect
        SDL_RenderFillRect(renderer, &inner_rect);
    }

    {
        SDL_Rect inner_rect{ rect.x + radius, rect.y, rect.w - 2 * radius, radius };

        // Render rect
        SDL_RenderFillRect(renderer, &inner_rect);
    }

    {
        SDL_Rect inner_rect{ rect.x + radius, rect.y + rect.h - radius, rect.w - 2 * radius, radius };

        // Render rect
        SDL_RenderFillRect(renderer, &inner_rect);
    }

    {
        SDL_Point point = { rect.x + radius, rect.y + radius };
        filledPieRGBA(renderer, static_cast<Sint16>(point.x), static_cast<Sint16>(point.y), 
            static_cast<Sint16>(radius), 180, 270, color.r, color.g, color.b, color.a);
    }

    {
        SDL_Point point = { rect.x + rect.w - radius - 1, rect.y + radius };
        filledPieRGBA(renderer, static_cast<Sint16>( point.x), static_cast<Sint16>(point.y), 
            static_cast<Sint16>(radius), 270, 360, color.r, color.g, color.b, color.a);
    }

    {
        SDL_Point point = { rect.x + radius, rect.y + rect.h - radius - 1 };
        filledPieRGBA(renderer, static_cast<Sint16>(point.x), static_cast<Sint16>(point.y),
            static_cast<Sint16>(radius), 90, 180, color.r, color.g, color.b, color.a);
    }

    {
        SDL_Point point = { rect.x + rect.w - radius - 1,  rect.y + rect.h - radius - 1 };
        filledPieRGBA(renderer, static_cast<Sint16>(point.x), static_cast<Sint16>(point.y),
            static_cast<Sint16>(radius), 0, 90, color.r, color.g, color.b, color.a);
    }
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