#pragma once

#include <SDL2/SDL.h>
#include <unordered_map>
#include <mutex>

void FilledRoundRect(SDL_Renderer* renderer, const SDL_Rect& rect, int radius, const SDL_Color& color);

void ThickFilledRoundRect(SDL_Renderer* renderer, const SDL_Rect& original_rect, int original_radius, int thickness, const SDL_Color& color1, const SDL_Color& color2);

void DrawRoundRect(SDL_Renderer* renderer, const SDL_Rect& rect, int radius, const SDL_Color color);

namespace textures
{
    void ResetAllTextures();
}

class RoundRectPainter
{
public:
    RoundRectPainter() {}

    RoundRectPainter(const RoundRectPainter&) noexcept {}
    RoundRectPainter& operator=(const RoundRectPainter&) noexcept {}

    RoundRectPainter(RoundRectPainter&& other) noexcept;
    RoundRectPainter& operator=(RoundRectPainter&& other) noexcept;

    ~RoundRectPainter();

    void Draw(SDL_Renderer* renderer, const SDL_Rect rect, int radius, const SDL_Color& color);

    void DropTexture();
    void DropTextureNoLock();

private:
    void ReCreateArcTexture(SDL_Renderer* renderer);

    SDL_Color stored_color{ 0,0,0,0 };
    int stored_radius{ 0 };
    SDL_Texture* stored_arc_texture{ nullptr };
};

void ThickFilledRoundRect(SDL_Renderer* renderer, const SDL_Rect& original_rect, int original_radius, int thickness, const SDL_Color& color1, const SDL_Color& color2,
    RoundRectPainter& outer, RoundRectPainter& inner);
