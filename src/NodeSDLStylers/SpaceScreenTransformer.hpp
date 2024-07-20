#pragma once
#include "SDL_Framework/SDL_headers.h"
#include "NodeModels/Utils.hpp"

namespace node
{
    SDL_Rect ToSDLRect(const model::Rect& rect);
    SDL_Point ToSDLPoint(const model::Point& point);
    class SpaceScreenTransformer
    {
    public:
        SpaceScreenTransformer(const SDL_Rect& screen, const model::Rect& space);
        SpaceScreenTransformer();
        model::Point ScreenToSpacePoint(const SDL_Point& p) const noexcept;
        SDL_Point SpaceToScreenPoint(const model::Point& p) const noexcept;
        model::Point ScreenToSpaceVector(const SDL_Point& p) const noexcept;
        SDL_Point SpaceToScreenVector(const model::Point& p) const noexcept;
        node::model::Rect ScreenToSpaceRect(const SDL_Rect& rect) const noexcept;
        SDL_Rect SpaceToScreenRect(const model::Rect& rect) const noexcept;
    private:
        SDL_Rect m_screenRect;
        model::Rect m_spaceRect;
    };

}