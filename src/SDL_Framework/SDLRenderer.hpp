#pragma once
#include <SDL_Framework/SDL_headers.h>
#include <vector>

namespace SDL
{
    class Renderer;
}
void swap(SDL::Renderer& first, SDL::Renderer& second) noexcept;

namespace SDL
{
    class RenderClip
    {
    public:
        RenderClip(Renderer& renderer, const SDL_Rect& rect);
        RenderClip(const RenderClip&) = delete;
        RenderClip(RenderClip&&);
        RenderClip& operator=(RenderClip&&);
        RenderClip& operator=(RenderClip&) = delete;
        ~RenderClip();
    private:
        Renderer* m_renderer;
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
    private:
        friend class RenderClip;
        void AddClipRect(const SDL_Rect& rect);
        void PopClipRect();
        void ClipLastTwoRects();
        SDL_Renderer* p_renderer = nullptr;
        std::vector<SDL_Rect> m_clip_rects;
    };
}
