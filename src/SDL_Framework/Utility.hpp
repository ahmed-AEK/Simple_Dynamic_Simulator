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

class DroppableTexture
{
public:
    DroppableTexture() {}

    DroppableTexture(DroppableTexture&& other) noexcept;
    DroppableTexture& operator=(DroppableTexture&& other) noexcept;

    ~DroppableTexture();

    void DropTexture();
    void DropTextureNoLock();
    void SetTexture(SDLTexture texture);
    SDL_Texture* GetTexture();
    operator bool() const { return m_stored_texture != nullptr; }
private:
    SDLTexture m_stored_texture{ nullptr };

};

class RoundRectPainter
{
public:
    RoundRectPainter() {}

    RoundRectPainter(const RoundRectPainter& other) noexcept
        :stored_color{other.stored_color}, stored_radius{other.stored_radius} {}
    RoundRectPainter& operator=(const RoundRectPainter& other) noexcept { 
        stored_color = other.stored_color;
        stored_radius = other.stored_radius;
        return *this;
    }

    RoundRectPainter(RoundRectPainter&& other) noexcept = default;
    RoundRectPainter& operator=(RoundRectPainter&& other) noexcept = default;

    void Draw(SDL_Renderer* renderer, const SDL_Rect rect, int radius, const SDL_Color& color);

private:
    void ReCreateArcTexture(SDL_Renderer* renderer);

    DroppableTexture m_arc_texture;
    SDL_Color stored_color{ 0,0,0,0 };
    int stored_radius{ 0 };
};

void ThickFilledRoundRect(SDL_Renderer* renderer, const SDL_Rect& original_rect, int original_radius, int thickness, const SDL_Color& color1, const SDL_Color& color2,
    RoundRectPainter& outer, RoundRectPainter& inner);

class TextPainter
{
public:
    explicit TextPainter(TTF_Font* font) :m_font{ font } {}
    TextPainter(const TextPainter& other) : m_text{ other.m_text }, m_font{ other.m_font } {}
    TextPainter& operator=(const TextPainter& other) { m_font = other.m_font; m_text = other.m_text; return *this; }
    TextPainter& operator=(TextPainter&&) = default;
    TextPainter(TextPainter&&) = default;

    void Draw(SDL_Renderer* renderer, const SDL_Point point, const SDL_Color color);
    SDL_Rect GetRect(SDL_Renderer* renderer, const SDL_Color color);

    void SetText(std::string text);
    void SetFont(TTF_Font* font);
    TTF_Font* GetFont() const { return m_font; }

private:
    void ReCreateTexture(SDL_Renderer* renderer);
    void AssureTexture(SDL_Renderer* renderer, const SDL_Color& color);

    DroppableTexture m_text_texture;
    std::string m_text;
    TTF_Font* m_font;
    SDL_Color m_stored_color{ 0,0,0,0 };

};