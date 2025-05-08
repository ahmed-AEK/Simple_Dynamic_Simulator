#pragma once
#include <SDL_Framework/SDL_headers.h>
#include "SDL_Framework/IntrusivePtr.hpp"
#include <vector>
#include <array>
#include <bitset>
#include <optional>
#include <atomic>

namespace SDL
{
    class Renderer;
}
void swap(SDL::Renderer& first, SDL::Renderer& second) noexcept;


enum class ColorRole : uint8_t
{
    None,
    frame_background,
    frame_outline,
    frame_background_alternate,
    frame_outline_alternate,
    dialog_background,
    dialog_outline,
    dialog_title_background,
    text_normal,
    btn_normal,
    btn_disable,
    btn_hover,
    btn_outline,
    scrollbar,
    red_close,
    red_close_clicked,
    blue_select,
    block_outline,
    block_outline_selected,
    block_background,
    input_socket,
    output_socket,
    netnode_normal,
    netnode_selected,
    netsegment_normal,
    netsegment_selected,
    COLOR_ROLE_COUNT,
};

struct ColorEntry
{
    ColorRole role;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class ColorTable;


class ColorTable
{
public:

    using ColorRole = ::ColorRole;
    using ColorEntry = ::ColorEntry;
    struct Color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    static constexpr size_t COLORS_COUNT = static_cast<size_t>(ColorRole::COLOR_ROLE_COUNT);

    ColorTable() = default;
    ColorTable(std::initializer_list<ColorEntry> colors);
    ColorTable(const ColorTable& other)
        :m_inplace_colors{other.m_inplace_colors}, m_valid_roles{other.m_valid_roles}
    {
    }
    ColorTable& operator=(const ColorTable& other)
    {
        if (this == &other) { return *this; }
        m_inplace_colors = other.m_inplace_colors;
        m_valid_roles = other.m_valid_roles;
        return *this;
    }

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
    int get_refcount() const
    {
        return m_refcount.load(std::memory_order_relaxed);
    }

    void SetColor(ColorRole role, Color color);
    std::optional<Color> GetColor(ColorRole color_role) const;
private:
    mutable std::atomic<int32_t> m_refcount = 1;
    std::array<Color, COLORS_COUNT> m_inplace_colors{};
    std::bitset<COLORS_COUNT> m_valid_roles{};
};

using ColorTablePtr = IntrusivePtr<ColorTable>;

class ColorNode;
using ColorNodePtr = IntrusivePtr<ColorNode>;

class ColorNode
{
public:
    explicit ColorNode(ColorNodePtr parent, const ColorTable& current);
    explicit ColorNode(const ColorTable& current);
    ColorNode() = default;

    ColorNode(const ColorNode& other) = delete;
    ColorNode& operator=(const ColorNode& other) = delete;
    void SetColor(ColorRole role, ColorTable::Color color);
    std::optional<ColorTable::Color> GetColor(ColorRole color_role) const;

    ColorNodePtr GetParent() const
    {
        return m_parent;
    }
    const ColorTable& GetTable() const
    {
        return m_current;
    }
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
    int get_refcount() const
    {
        return m_refcount.load(std::memory_order_relaxed);
    }
    bool IsDarkMode() const;
    void SetDarkMode(bool value = true) { m_is_darkmode = value; }
    [[nodiscard]] ColorNodePtr CloneTable() const;
private:
    mutable std::atomic<int32_t> m_refcount = 1;
    std::optional<bool> m_is_darkmode = {};
    ColorNodePtr m_parent{};
    ColorTable m_current{};
};

class ColorPalette
{
public:
    explicit ColorPalette(ColorNodePtr node);
    explicit ColorPalette(ColorNodePtr parent, const ColorTable& current);
    explicit ColorPalette(const ColorTable& current);
    ColorPalette() = default;
    void SetColor(ColorRole role, ColorTable::Color color);
    void SetTable(const ColorTable& table);
    std::optional<ColorTable::Color> GetColor(ColorRole color_role) const;
    friend bool operator==(const ColorPalette& lhs, const ColorPalette& rhs)
    {
        return lhs.m_node == rhs.m_node;
    }
    bool empty() const { return m_node.get() == nullptr; }

    bool IsDarkMode() const 
    { 
        if (!m_node) { return false; }  
        return m_node->IsDarkMode();
    }
    void SetDarkMode(bool value = true) 
    { 
        if (!m_node) { return; }
        m_node->SetDarkMode(value); 
    }
private:
    ColorNodePtr m_node{};
};

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
        PaletteScope(Renderer& renderer, ColorPalette palette);
        PaletteScope(const PaletteScope&) = delete;
        PaletteScope(PaletteScope&&) noexcept;
        PaletteScope& operator=(const PaletteScope&) = delete;
        PaletteScope& operator=(PaletteScope&&) noexcept;
        ~PaletteScope();
    private:
        Renderer* m_renderer{};
        ColorPalette m_palette{};
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
        [[nodiscard]] PaletteScope SetColorPalette(ColorPalette palette);
        SDL_Color GetColor(ColorRole color_type);
        bool IsDarkMode() const;
    private:
        friend class RenderClip;
        friend class PaletteScope;
        void AddPalette(ColorPalette palette);
        void PopPalette(ColorPalette palette);
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
        std::vector<ColorPalette> m_color_palettes;
    };
}
