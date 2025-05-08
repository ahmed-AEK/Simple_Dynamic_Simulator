#pragma once
#include <SDL_Framework/SDL_headers.h>
#include "SDL_Framework/IntrusivePtr.hpp"
#include <vector>
#include <array>

namespace SDL
{
    class Renderer;
}
void swap(SDL::Renderer& first, SDL::Renderer& second) noexcept;


enum class ColorRole : uint8_t
{
    None,
    text_normal,
    btn_normal,
    btn_hover,
    btn_outline,
};

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4324) // type size bigger due to alignment
#endif

class alignas(64) ColorPalette
{
public:

    using ColorRole = ::ColorRole;
    struct Color
    {
        ColorRole type;
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    ColorPalette(IntrusivePtr<ColorPalette> parent);
    ColorPalette(std::initializer_list<Color> colors);
    ColorPalette(std::initializer_list<Color> colors, IntrusivePtr<ColorPalette> parent);
    void increment_ref() const
    {
        m_refcount.fetch_add(1, std::memory_order_relaxed);
    }
    void decrement_ref() const
    {
        if (m_refcount.fetch_add(-1, std::memory_order_release) == int32_t{ 1 })
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            delete this;
        }
    }

    void SetColor(Color color);
    Color GetColor(ColorRole color_type) const;
private:
    IntrusivePtr<ColorPalette> m_parent = nullptr;
    mutable std::atomic<int32_t> m_refcount = 1;
    std::array<Color, 7> m_inplace_colors{};
    std::vector<Color> m_heap_colors;
};

using ColorPalettePtr = IntrusivePtr<ColorPalette>;

#ifdef _WIN32
#pragma warning(pop)
#endif

namespace SDL
{
    class RenderClip
    {
    public:
        RenderClip(Renderer& renderer, const SDL_Rect& rect);
        RenderClip(const RenderClip&) = delete;
        RenderClip(RenderClip&&) noexcept;
        RenderClip& operator=(RenderClip&&) noexcept;
        RenderClip& operator=(RenderClip&) = delete;
        ~RenderClip();
        bool isClipped() const { return m_success; }
        operator bool() const { return m_success; }
    private:
        Renderer* m_renderer;
        bool m_success;
    };

    class PaletteScope
    {
    public:
        PaletteScope();
        PaletteScope(Renderer& renderer, ColorPalettePtr palette);
        PaletteScope(const PaletteScope&) = delete;
        PaletteScope(PaletteScope&&) noexcept;
        PaletteScope& operator=(const PaletteScope&) = delete;
        PaletteScope& operator=(PaletteScope&&) noexcept;
        ~PaletteScope();
    private:
        Renderer* m_renderer{};
        ColorPalette* m_palette{};
    };

    class Renderer
    {
    public:
        Renderer();
        ~Renderer() noexcept;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(const Renderer&) = delete;
        Renderer& operator=(Renderer&&) noexcept;
        Renderer(Renderer&&) noexcept;
        operator SDL_Renderer* () const noexcept { return p_renderer; };
        bool Init(SDL_Window* wnd);
        friend void ::swap(Renderer& first, Renderer& second) noexcept;
        [[nodiscard]] RenderClip ClipRect(const SDL_Rect& rect);
        [[nodiscard]] SDL_Rect GetClipRect() const;
        [[nodiscard]] PaletteScope SetColorPalette(ColorPalettePtr palette);
        SDL_Color GetColor(ColorPalette::ColorRole color_type);
    private:
        friend class RenderClip;
        friend class PaletteScope;
        void AddPalette(ColorPalettePtr palette);
        void PopPalette(ColorPalette* palette);
        bool AddClipRect(const SDL_Rect& rect);
        void PopClipRect();
        void ClipLastTwoRects();
        void ReApplyLastClip();
        SDL_Renderer* p_renderer = nullptr;

        struct ClipData
        {
            SDL_Rect viewport_rect;
            SDL_Rect clip_rect;
        };
        std::vector<ClipData> m_viewport_rects;
        std::vector<IntrusivePtr<ColorPalette>> m_color_palettes;
    };
}
