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
        p_renderer = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
        if (!p_renderer)
        {
            SDL_Log("Couldn't initialize SDL Renderer");
            return false;
        }
        return true;
    }
}


