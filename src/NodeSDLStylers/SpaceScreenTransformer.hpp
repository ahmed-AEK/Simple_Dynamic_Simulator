#pragma once
#include "SDL_Framework/SDL_headers.h"
#include "NodeModels/Utils.hpp"

namespace node
{
    SDL_FRect ToSDLRect(const model::Rect& rect);
    SDL_FPoint ToSDLPoint(const model::Point& point);
    class SpaceScreenTransformer
    {
    public:
        SpaceScreenTransformer(const SDL_FRect& screen, const model::Rect& space);
        SpaceScreenTransformer();
        model::Point ScreenToSpacePoint(const SDL_FPoint& p) const noexcept;
        SDL_FPoint SpaceToScreenPoint(const model::Point& p) const noexcept;
        model::Point ScreenToSpaceVector(const SDL_FPoint& p) const noexcept;
        SDL_FPoint SpaceToScreenVector(const model::Point& p) const noexcept;
        node::model::Rect ScreenToSpaceRect(const SDL_FRect& rect) const noexcept;
        SDL_FRect SpaceToScreenRect(const model::Rect& rect) const noexcept;
        SpaceScreenTransformer WithZeroOffset() const
        {
            SpaceScreenTransformer other = *this;
            other.m_screenRect.x = 0;
            other.m_screenRect.y = 0;
            other.m_spaceRect.x = 0;
            other.m_spaceRect.y = 0;
            return other;
        }
    private:
        SDL_FRect m_screenRect;
        model::Rect m_spaceRect;
    };

}