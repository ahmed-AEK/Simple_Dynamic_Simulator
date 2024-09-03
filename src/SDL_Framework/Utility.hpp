#pragma once

#include "SDL_Framework/SDL_headers.h"
#include "SDL_Framework/SDLCPP.hpp"
#include <unordered_map>
#include <mutex>
#include <optional>

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

class TextPainter
{
public:
    explicit TextPainter(TTF_Font* font) :m_font{ font } {}
    TextPainter(const TextPainter& other) :m_font{ other.m_font }, m_text{ other.m_text } {}
    TextPainter& operator=(const TextPainter& other) { m_font = other.m_font; m_text = other.m_text; }
    ~TextPainter();

    void Draw(SDL_Renderer* renderer, const SDL_Point point, const SDL_Color color);
    SDL_Rect GetRect(SDL_Renderer* renderer, const SDL_Color color);

    void SetText(std::string text);
    void SetFont(TTF_Font* font);
    TTF_Font* GetFont() const { return m_font; }

    void DropTexture();
    void DropTextureNoLock();
private:
    void ReCreateTexture(SDL_Renderer* renderer);
    void AssureTexture(SDL_Renderer* renderer, const SDL_Color& color);

    std::string m_text;
    TTF_Font* m_font;
    SDLTexture m_texture;
    SDL_Color m_stored_color{ 0,0,0,0 };

};