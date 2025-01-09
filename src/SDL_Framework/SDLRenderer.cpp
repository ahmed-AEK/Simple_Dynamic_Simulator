#include "SDLRenderer.hpp"

#include <utility>
#include <cassert>

void swap(SDL::Renderer& first, SDL::Renderer& second) noexcept
{
    std::swap(first.p_renderer, second.p_renderer); 
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
            if (strcmp("direct3d", driver) == 0)
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
            driver = "direct3d";
        }
        else if (opengl_found)
        {
            driver = "opengl";
        }

        if (driver)
        {
            SDL_Log("using driver: %s", driver);
        }

        p_renderer = SDL_CreateRenderer(wnd, driver);
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

    void Renderer::AddClipRect(const SDL_Rect& rect)
    {
        assert(p_renderer);
        auto new_rect = rect;
        if (m_clip_rects.size())
        {
            auto&& last_clip_rect = m_clip_rects.back();
            new_rect.x += last_clip_rect.x;
            new_rect.y += last_clip_rect.y;
        }
        m_clip_rects.push_back(new_rect);
        [[maybe_unused]] auto result = SDL_SetRenderViewport(p_renderer, &new_rect);
        assert(result);
        ClipLastTwoRects();
    }

    void Renderer::PopClipRect()
    {
        m_clip_rects.pop_back();
        if (m_clip_rects.size())
        {
            SDL_SetRenderViewport(p_renderer, &m_clip_rects.back());
        }
        else
        {
            SDL_SetRenderViewport(p_renderer, nullptr);
        }
        ClipLastTwoRects();
    }

    void Renderer::ClipLastTwoRects()
    {
        if (m_clip_rects.size() < 2)
        {
            SDL_SetRenderClipRect(p_renderer, nullptr);
            return;
        }

        auto& last_rect = *(m_clip_rects.end() - 1);
        auto& before_last_rect = *(m_clip_rects.end() - 2);
        SDL_Rect clip_rect;
        auto done = SDL_GetRectIntersection(&last_rect, &before_last_rect, &clip_rect);
        if (done)
        {
            clip_rect.x -= last_rect.x;
            clip_rect.y -= last_rect.y;
            SDL_SetRenderClipRect(p_renderer, &clip_rect);
        }
        else
        {
            SDL_SetRenderClipRect(p_renderer, nullptr);
        }
    }

    RenderClip::RenderClip(Renderer& renderer, const SDL_Rect& rect)
        :m_renderer{&renderer}
    {
        renderer.AddClipRect(rect);
    }

    RenderClip::RenderClip(RenderClip&& other)
    {
        m_renderer = other.m_renderer;
        other.m_renderer = nullptr;
    }

    RenderClip& RenderClip::operator=(RenderClip&& other)
    {
        m_renderer = other.m_renderer;
        other.m_renderer = nullptr;
        return *this;
    }

    RenderClip::~RenderClip()
    {
        if (m_renderer)
        {
            m_renderer->PopClipRect();
        }
    }
}


