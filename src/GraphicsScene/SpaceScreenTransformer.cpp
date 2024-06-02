#include "SpaceScreenTransformer.hpp"

node::SpaceScreenTransformer::SpaceScreenTransformer(const SDL_Rect& screen, const SDL_Rect& space)
    :m_screenRect(screen), m_spaceRect(space)
{
}

node::SpaceScreenTransformer::SpaceScreenTransformer()
    :m_screenRect({0,0,100,100}), m_spaceRect({0,0,100,100})
{
}

SDL_Point node::SpaceScreenTransformer::ScreenToSpacePoint(const SDL_Point& p) const noexcept
{
    return SDL_Point{
    static_cast<int>(
    static_cast<double>(p.x - m_screenRect.x) / (m_screenRect.w) * m_spaceRect.w + m_spaceRect.x),
    static_cast<int>(
    static_cast<double>(p.y - m_screenRect.y) / (m_screenRect.h) * m_spaceRect.h + m_spaceRect.y)
    };
}

SDL_Point node::SpaceScreenTransformer::SpaceToScreenPoint(const SDL_Point& p) const noexcept
{
    return SDL_Point{ static_cast<int>(
    static_cast<double>(p.x - m_spaceRect.x) / (m_spaceRect.w) * m_screenRect.w + m_screenRect.x),
    static_cast<int>(
    static_cast<double>(p.y - m_spaceRect.y) / (m_spaceRect.h) * m_screenRect.h + m_screenRect.y)
    };
}

SDL_Point node::SpaceScreenTransformer::ScreenToSpaceVector(const SDL_Point& p) const noexcept
{
    return SDL_Point{
    static_cast<int>(
    static_cast<double>(p.x) / (m_screenRect.w) * m_spaceRect.w),
    static_cast<int>(
    static_cast<double>(p.y) / (m_screenRect.h) * m_spaceRect.h)
    };
}

SDL_Point node::SpaceScreenTransformer::SpaceToScreenVector(const SDL_Point& p) const noexcept
{
    return SDL_Point{ static_cast<int>(
        static_cast<double>(p.x) / (m_spaceRect.w) * m_screenRect.w),
        static_cast<int>(
        static_cast<double>(p.y) / (m_spaceRect.h) * m_screenRect.h)
    };
}

SDL_Rect node::SpaceScreenTransformer::ScreenToSpaceRect(const SDL_Rect& rect) const noexcept
{
    SDL_Point p1 = ScreenToSpacePoint({ rect.x, rect.y });
    SDL_Point p2 = ScreenToSpaceVector({ rect.w, rect.h });
    return { p1.x, p1.y, p2.x, p2.y };
}

SDL_Rect node::SpaceScreenTransformer::SpaceToScreenRect(const SDL_Rect& rect) const noexcept
{
    SDL_Point p1 = SpaceToScreenPoint({ rect.x, rect.y });
    SDL_Point p2 = SpaceToScreenVector({ rect.w, rect.h });
    return { p1.x, p1.y, p2.x, p2.y };
}
