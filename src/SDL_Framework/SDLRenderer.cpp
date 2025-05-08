#include "SDLRenderer.hpp"

#include <utility>
#include <cassert>
#include <ranges>

void swap(SDL::Renderer& first, SDL::Renderer& second) noexcept
{
    std::swap(first.p_renderer, second.p_renderer); 
}

ColorTable::ColorTable(std::initializer_list<ColorEntry> colors)
{
    for (auto& color : colors)
    {
        SetColor(color.role, {color.r, color.g, color.b});
    }
}

void ColorTable::SetColor(ColorRole role, Color color)
{
    assert(role != ColorRole::None);
    const size_t position = static_cast<size_t>(role);
    assert(position < m_inplace_colors.size());
    m_valid_roles[position] = true;
    m_inplace_colors[position] = color;
}

std::optional<ColorTable::Color> ColorTable::GetColor(ColorRole color_role) const
{
    const size_t position = static_cast<size_t>(color_role);
    assert(position < m_inplace_colors.size());
    if (!m_valid_roles[position])
    {
        return {};
    }
    return m_inplace_colors[position];
}

ColorNode::ColorNode(ColorNodePtr parent, const ColorTable& current)
    :m_parent{std::move(parent)}, m_current(std::move(current))
{

}
ColorNode::ColorNode(const ColorTable& current)
    :m_current{std::move(current)}
{

}

void ColorNode::SetColor(ColorRole role, ColorTable::Color color)
{
    m_current.SetColor(role, color);
}

bool ColorNode::IsDarkMode() const
{
    if (m_is_darkmode)
    {
        return *m_is_darkmode;
    }
    auto* parent = m_parent.get();
    while (parent)
    {
        if (parent->m_is_darkmode)
        {
            return *parent->m_is_darkmode;
        }
        parent = parent->GetParent().get();
    }
    return false;
}

std::optional<ColorTable::Color> ColorNode::GetColor(ColorRole color_role) const
{

    if (auto color = m_current.GetColor(color_role))
    {
        return color;
    }
    
    auto parent = m_parent;
    while (parent)
    {
        if (auto color = parent->m_current.GetColor(color_role))
        {
            return color;
        }
        parent = parent->m_parent;
    }
    return {};
}

ColorNodePtr ColorNode::CloneTable() const
{
    return make_intrusive<ColorNode>(m_parent, m_current);
}

ColorPalette::ColorPalette(ColorNodePtr node)
    :m_node{std::move(node)}
{
}

ColorPalette::ColorPalette(ColorNodePtr parent, const ColorTable& current)
    :m_node{make_intrusive<ColorNode>(std::move(parent), std::move(current))}
{
}

ColorPalette::ColorPalette(const ColorTable& current)
    :m_node{make_intrusive<ColorNode>(std::move(current))}
{
}

void ColorPalette::SetColor(ColorRole role, ColorTable::Color color)
{
    if (!m_node)
    {
        m_node = make_intrusive<ColorNode>();
    }
    if (m_node->get_refcount() != 1)
    {
        m_node = m_node->CloneTable();
    }
    m_node->SetColor(role, color);
}

void ColorPalette::SetTable(const ColorTable& table)
{
    if (!m_node)
    {
        m_node = make_intrusive<ColorNode>(std::move(table));
        return;
    }
    m_node = make_intrusive<ColorNode>(m_node->GetParent(), std::move(table));
}

std::optional<ColorTable::Color> ColorPalette::GetColor(ColorRole color_role) const
{
    if (!m_node)
    {
        return {};
    }
    return m_node->GetColor(color_role);
}

namespace SDL
{
    Renderer::Renderer()
    : p_renderer(nullptr)
    {
    }

    Renderer::~Renderer() noexcept
    {
        if (p_renderer)
        {
            SDL_DestroyRenderer(p_renderer);
        }
    }

    Renderer& Renderer::operator=(Renderer && other) noexcept
    {
        swap(*this, other);
        return *this; 
    }

    Renderer::Renderer(Renderer && other) noexcept
    : p_renderer(nullptr)
    {
        swap(*this, other);
    }

    bool Renderer::Init(SDL_Window* wnd)
    {
        int num_drivers = SDL_GetNumRenderDrivers();
        bool direct3d_found = false;
        bool opengl_found = false;
        for (int i = 0; i < num_drivers; i++)
        {
            const char* driver = SDL_GetRenderDriver(i);
            // SDL_Log("driver %d: %s", i, driver);
            if (strcmp("direct3d11", driver) == 0)
            {
                direct3d_found = true;
            }
            if (strcmp("opengl", driver) == 0)
            {
                opengl_found = true;
            }
        }
        const char* driver = nullptr;
        if (direct3d_found)
        {
            driver = "direct3d11";
        }
        else if (opengl_found)
        {
            driver = "opengl";
        }

        if (driver)
        {
            SDL_SetHint(SDL_HINT_RENDER_DRIVER, driver);
            SDL_Log("using driver: %s", driver);
        }

        p_renderer = SDL_CreateRenderer(wnd, nullptr);
        SDL_SetRenderVSync(p_renderer, 1);
        if (!p_renderer)
        {
            SDL_Log("Couldn't initialize SDL Renderer");
            return false;
        }
        return true;
    }
    RenderClip Renderer::ClipRect(const SDL_Rect& rect)
    {
        return RenderClip{ *this, rect };
    }

    SDL_Rect Renderer::GetClipRect() const
    {
        if (m_viewport_rects.size())
        {
            return m_viewport_rects.back().clip_rect;
        }
        return SDL_Rect{ 0, 0, 10000, 10000 };
    }

    bool Renderer::AddClipRect(const SDL_Rect& rect)
    {
        assert(p_renderer);
        auto new_viewport_rect{ rect };
        SDL_Rect new_clip_rect{ rect };
        bool success = true;
        if (m_viewport_rects.size())
        {
            auto&& last_clip = m_viewport_rects.back();
            auto last_clip_rect = last_clip.clip_rect;

            // convert last_clip_rect from local to global coords
            last_clip_rect.x += last_clip.viewport_rect.x;
            last_clip_rect.y += last_clip.viewport_rect.y;
            
            // convert new_clip_rect from local to global coords
            new_clip_rect.x += last_clip.viewport_rect.x;;
            new_clip_rect.y += last_clip.viewport_rect.y;
            
            // get intersection
            SDL_Rect intersect_rect;
            success = SDL_GetRectIntersection(&last_clip_rect, &new_clip_rect, &intersect_rect);
            if (!success)
            {
                return false;
            }
            new_clip_rect = intersect_rect; // new_clip_rect is in global coords
            // convert new_viewport_rect from local to global coords
            new_viewport_rect.x += last_clip.viewport_rect.x;
            new_viewport_rect.y += last_clip.viewport_rect.y;
            
            // convert new_clip_rect from global to new local corrds
            new_clip_rect.x -= new_viewport_rect.x;
            new_clip_rect.y -= new_viewport_rect.y;
        }
        else
        {
            new_clip_rect.x = 0;
            new_clip_rect.y = 0;
        }
        success = SDL_SetRenderViewport(p_renderer, &new_viewport_rect);
        if (!success)
        {
            ReApplyLastClip();
            return false;
        }
        success = SDL_SetRenderClipRect(p_renderer, &new_clip_rect);
        if (!success)
        {
            ReApplyLastClip();
            return false;
        }
        m_viewport_rects.push_back({new_viewport_rect, new_clip_rect});
        return true;
    }

    void Renderer::ReApplyLastClip()
    {
        if (m_viewport_rects.size())
        {
            auto&& last_clip = m_viewport_rects.back();
            SDL_SetRenderViewport(p_renderer, &last_clip.viewport_rect);
            SDL_SetRenderClipRect(p_renderer, &last_clip.clip_rect);
        }
        else
        {
            SDL_SetRenderViewport(p_renderer, nullptr);
            SDL_SetRenderClipRect(p_renderer, nullptr);
        }
    }
    void Renderer::PopClipRect()
    {
        m_viewport_rects.pop_back();
        ReApplyLastClip();
    }

    RenderClip::RenderClip(Renderer& renderer, const SDL_Rect& rect)
        :m_renderer{&renderer}
    {
        m_success = renderer.AddClipRect(rect);
    }

    RenderClip::RenderClip(RenderClip&& other) noexcept
    {
        m_renderer = std::exchange(other.m_renderer, nullptr);
        m_success = std::exchange(other.m_success, false);
    }

    RenderClip& RenderClip::operator=(RenderClip&& other) noexcept
    {
        m_renderer = std::exchange(other.m_renderer, nullptr);
        m_success = std::exchange(other.m_success, false);
        return *this;
    }

    RenderClip::~RenderClip()
    {
        if (m_renderer && m_success)
        {
            m_renderer->PopClipRect();
        }
    }
    PaletteScope::PaletteScope()
    {
    }

    PaletteScope::PaletteScope(Renderer& renderer, ColorPalette palette)
        : m_renderer{ &renderer }, m_palette { palette }
    {
        renderer.AddPalette(std::move(palette));
    }
    PaletteScope::PaletteScope(PaletteScope&& other) noexcept
        : m_renderer{std::exchange(other.m_renderer, nullptr)},
        m_palette{std::exchange(other.m_palette, ColorPalette{})}
    {
    }

    PaletteScope& PaletteScope::operator=(PaletteScope&& other) noexcept
    {
        if (this != &other)
        {
            m_renderer = std::exchange(other.m_renderer, nullptr);
            m_palette = std::exchange(other.m_palette, ColorPalette{});
        }
        return *this;
    }

    PaletteScope::~PaletteScope()
    {
        if (m_renderer)
        {
            m_renderer->PopPalette(m_palette);
        }
    }

    PaletteScope Renderer::SetColorPalette(ColorPalette palette)
    {
        if (palette.empty())
        {
            return PaletteScope{};
        }
        if (m_color_palettes.size() == 0 || palette != m_color_palettes.back())
        {
            return PaletteScope{ *this, std::move(palette) };
        }
        return PaletteScope{};
    }

    SDL_Color Renderer::GetColor(ColorRole color_type)
    {
        constexpr SDL_Color error_color{ 255,8,255,255 };
        if (!m_color_palettes.size())
        {
            return error_color;
        }
        for (auto& palette : std::ranges::reverse_view(m_color_palettes))
        {
            auto color = palette.GetColor(color_type);
            if (color)
            {
                return { .r = color->r, .g = color->g, .b = color->b, .a = 255 };
            }
        }
        return error_color;
    }

    bool Renderer::IsDarkMode() const
    {
        if (!m_color_palettes.size())
        {
            return false;
        }
        return m_color_palettes.back().IsDarkMode();
    }

    void Renderer::AddPalette(ColorPalette palette)
    {
        m_color_palettes.push_back(std::move(palette));
    }

    void Renderer::PopPalette(ColorPalette palette)
    {
        assert(m_color_palettes.size());
        if (m_color_palettes.size() && palette == m_color_palettes.back())
        {
            m_color_palettes.pop_back();
        }
    }
}

