#pragma once

#include <SDL_Framework/SDL_headers.h>
#include <cstdint>
#include <memory>

inline auto SDL_Window_deleter=[](SDL_Window* ptr){
    SDL_DestroyWindow(ptr);
};

using SDLWindow = std::unique_ptr<SDL_Window, decltype(SDL_Window_deleter)>;

inline auto SDL_Renderer_deleter=[](SDL_Renderer* ptr)
{
    SDL_DestroyRenderer(ptr);
};

using SDLRenderer = std::unique_ptr<SDL_Renderer, decltype(SDL_Renderer_deleter)>;


inline auto SDL_TTF_deleter=[](TTF_Font* ptr)
{
    TTF_CloseFont(ptr);
};

using TTFFont = std::unique_ptr<TTF_Font, decltype(SDL_TTF_deleter)>;

inline auto SDL_Texture_deleter=[](SDL_Texture* ptr)
{
    SDL_DestroyTexture(ptr);
};

using SDLTexture = std::unique_ptr<SDL_Texture, decltype(SDL_Texture_deleter)>;

inline auto SDL_Surface_deleter=[](SDL_Surface* ptr)
{
    SDL_FreeSurface(ptr);
};

using SDLSurface = std::unique_ptr<SDL_Surface, decltype(SDL_Surface_deleter)>;

inline auto SDL_Cursor_deleter=[](SDL_Cursor* ptr)
{
    SDL_FreeCursor(ptr);
};

using SDLCursor = std::unique_ptr<SDL_Cursor, decltype(SDL_Cursor_deleter)>;