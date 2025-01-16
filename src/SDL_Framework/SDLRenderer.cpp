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

    RenderClip::RenderClip(RenderClip&& other)
    {
        m_renderer = std::exchange(other.m_renderer, nullptr);
        m_success = std::exchange(other.m_success, false);
    }

    RenderClip& RenderClip::operator=(RenderClip&& other)
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
}


