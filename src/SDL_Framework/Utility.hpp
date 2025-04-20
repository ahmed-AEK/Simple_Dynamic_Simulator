#pragma once

#include "SDL_Framework/SDL_headers.h"
#include "SDL_Framework/SDLCPP.hpp"
#include "SDL_Framework/SDLRenderer.hpp"
#include <unordered_map>
#include <mutex>
#include <optional>
#include <cmath>
#include <array>

void FilledRoundRect(SDL_Renderer* renderer, const SDL_FRect& rect, int radius, const SDL_Color& color);

void ThickFilledRoundRect(SDL_Renderer* renderer, const SDL_FRect& original_rect, int original_radius, float thickness, const SDL_Color& color1, const SDL_Color& color2);

void DrawRoundRect(SDL_Renderer* renderer, const SDL_FRect& rect, int radius, const SDL_Color color);

inline SDL_FRect ToFRect(const SDL_Rect& rect)
{
    SDL_FRect rect2;
    SDL_RectToFRect(&rect, &rect2);
    return rect2;
}

inline SDL_Rect ToRectCeil(const SDL_FRect& rect)
{
    return
    {
        static_cast<int>(rect.x),
        static_cast<int>(rect.y),
        static_cast<int>(std::ceil(rect.w)),
        static_cast<int>(std::ceil(rect.h)),
    };
}

inline SDL_Rect ToRect(const SDL_FRect& rect)
{
    return
    {
        static_cast<int>(rect.x),
        static_cast<int>(rect.y),
        static_cast<int>(rect.w),
        static_cast<int>(rect.h),
    };
}

inline SDL_FColor ToFColor(const SDL_Color& color)
{
    return
    {
        static_cast<float>(color.r)/255,
        static_cast<float>(color.g)/255,
        static_cast<float>(color.b)/255,
        static_cast<float>(color.a)/255,
    };
}

namespace textures
{
    void ResetAllTextures();
}

class DroppableTexture
{
public:
    DroppableTexture() = default;
    explicit DroppableTexture(SDLTexture&& texture);
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

    RoundRectPainter(const RoundRectPainter& other) = default;
    RoundRectPainter& operator=(const RoundRectPainter& other) = default;

    RoundRectPainter(RoundRectPainter&& other) = default;
    RoundRectPainter& operator=(RoundRectPainter&& other) = default;

    void Draw(SDL_Renderer* renderer, SDL_FRect rect, int radius, const SDL_Color& color);

    void SetDrawSides(bool NW, bool NE, bool SE, bool SW);
private:
    void ReCreateArcTexture(SDL_Renderer* renderer);

    std::shared_ptr<DroppableTexture> m_arc_texture;
    SDL_Color stored_color{ 0,0,0,0 };
    int stored_radius{ 0 };
    std::array<bool, 4> m_draw_sides{ true,true,true,true };
};

void ThickFilledRoundRect(SDL_Renderer* renderer, const SDL_FRect& original_rect, int original_radius, int thickness, const SDL_Color& color1, const SDL_Color& color2,
    RoundRectPainter& outer, RoundRectPainter& inner);

class TextPainter
{
public:
    explicit TextPainter(TTF_Font* font) :m_font{ font } {}
    TextPainter(const TextPainter& other) : m_text{ other.m_text }, m_font{ other.m_font } {}
    TextPainter& operator=(const TextPainter& other) { m_font = other.m_font; m_text = other.m_text; return *this; }
    TextPainter& operator=(TextPainter&&) = default;
    TextPainter(TextPainter&&) = default;

    void Draw(SDL_Renderer* renderer, const SDL_FPoint point, const SDL_Color color);
    SDL_FRect GetRect(SDL_Renderer* renderer, const SDL_Color color);

    void SetText(std::string_view text);
    void SetFont(TTF_Font* font);
    TTF_Font* GetFont() const { return m_font; }
    const std::string& GetText() const { return m_text; }
private:
    void ReCreateTexture(SDL_Renderer* renderer);
    void AssureTexture(SDL_Renderer* renderer, const SDL_Color& color);

    DroppableTexture m_text_texture;
    std::string m_text;
    TTF_Font* m_font;
    SDL_Color m_stored_color{ 0,0,0,0 };

};


class TruncatedTextPainter
{
public:
    explicit TruncatedTextPainter(TTF_Font* font);
    void Draw(SDL::Renderer& renderer, const SDL_FPoint point, const SDL_Color color);
    int GetHeight() const;
    void SetPixelOffset(size_t offset);
    size_t GetPixelOffset() const { return m_pixel_offset; }
    void SetWidth(size_t width);
    size_t GetWidth() const { return m_width; }
    void SetText(std::string text);
    std::string_view GetText() const { return m_text; }
    TTF_Font* GetFont() const;
private:
    void UpdatePainter();
    std::string m_text;
    TextPainter m_painter;
    size_t m_pixel_offset = 0;
    int m_width = 100;
    uint16_t m_local_offset = 0;
    bool m_dirty = true;
};