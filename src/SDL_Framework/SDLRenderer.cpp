#include "SDLRenderer.hpp"
#include <utility>

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
    Renderer::operator SDL_Renderer *() const noexcept
    {
        return p_renderer;
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
}


