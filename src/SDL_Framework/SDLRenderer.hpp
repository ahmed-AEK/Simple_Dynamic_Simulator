#pragma once
#include <SDL_Framework/SDL_headers.h>

namespace SDL
{
    class Renderer;
}
void swap(SDL::Renderer& first, SDL::Renderer& second) noexcept;

namespace SDL
{
    class Renderer
    {
        public:
        Renderer();
        ~Renderer() noexcept;
        Renderer& operator=(Renderer&) = delete;
        Renderer(Renderer&) = delete;
        Renderer& operator=(Renderer&&) noexcept;
        Renderer(Renderer&&) noexcept;
        operator SDL_Renderer*() const noexcept;
        bool Init(SDL_Window* wnd);
        friend void ::swap(Renderer& first, Renderer& second) noexcept;
        private:
        SDL_Renderer* p_renderer;
    };
}
