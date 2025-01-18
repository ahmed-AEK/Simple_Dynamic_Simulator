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
        SpaceScreenTransformer(const SDL_FRect& screen_rect, const model::Rect& space_rect);
        SpaceScreenTransformer(const SDL_FPoint& screen_orig, const model::Point& space_orig, float space_to_screen_factor);
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
            other.m_screen_orig.x = 0;
            other.m_screen_orig.y = 0;
            other.m_space_orig.x = 0;
            other.m_space_orig.y = 0;
            return other;
        }
    private:
        SDL_FPoint m_screen_orig;
        model::Point m_space_orig;
        float m_space_to_screen_factor_x;
        float m_space_to_screen_factor_y;
    };

}