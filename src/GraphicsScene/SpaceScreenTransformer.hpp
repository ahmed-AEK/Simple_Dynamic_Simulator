#pragma once
#include "SDL2/SDL.h"

namespace node
{

class SpaceScreenTransformer
{
public:
    SpaceScreenTransformer(const SDL_Rect& screen, const SDL_Rect& space);
    SpaceScreenTransformer();
    SDL_Point ScreenToSpacePoint(const SDL_Point& p) const noexcept;
    SDL_Point SpaceToScreenPoint(const SDL_Point& p) const noexcept;
    SDL_Point ScreenToSpaceVector(const SDL_Point& p) const noexcept;
    SDL_Point SpaceToScreenVector(const SDL_Point& p) const noexcept;
    SDL_Rect ScreenToSpaceRect(const SDL_Rect& rect) const noexcept;
    SDL_Rect SpaceToScreenRect(const SDL_Rect& rect) const noexcept;
private:
    SDL_Rect m_screenRect;
    SDL_Rect m_spaceRect;
};

}