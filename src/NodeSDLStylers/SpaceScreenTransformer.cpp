#include "SpaceScreenTransformer.hpp"

node::SpaceScreenTransformer::SpaceScreenTransformer(const SDL_FRect& screen, const model::Rect& space)
    :m_screenRect(screen), m_spaceRect(space)
{
}

node::SpaceScreenTransformer::SpaceScreenTransformer()
    :m_screenRect({ 0,0,100,100 }), m_spaceRect({ 0,0,100,100 })
{
}

node::model::Point node::SpaceScreenTransformer::ScreenToSpacePoint(const SDL_FPoint& p) const noexcept
{
    return node::model::Point{
    static_cast<int>(
    static_cast<double>(p.x - m_screenRect.x) / (m_screenRect.w) * m_spaceRect.w + m_spaceRect.x),
    static_cast<int>(
    static_cast<double>(p.y - m_screenRect.y) / (m_screenRect.h) * m_spaceRect.h + m_spaceRect.y)
    };
}

SDL_FPoint node::SpaceScreenTransformer::SpaceToScreenPoint(const node::model::Point& p) const noexcept
{
    return SDL_FPoint{ static_cast<float>(
    static_cast<float>(p.x - m_spaceRect.x) / (m_spaceRect.w) * m_screenRect.w + m_screenRect.x),
    static_cast<float>(
    static_cast<float>(p.y - m_spaceRect.y) / (m_spaceRect.h) * m_screenRect.h + m_screenRect.y)
    };
}

node::model::Point node::SpaceScreenTransformer::ScreenToSpaceVector(const SDL_FPoint& p) const noexcept
{
    return model::Point{
    static_cast<int>(
    static_cast<double>(p.x) / (m_screenRect.w) * m_spaceRect.w),
    static_cast<int>(
    static_cast<double>(p.y) / (m_screenRect.h) * m_spaceRect.h)
    };
}

SDL_FPoint node::SpaceScreenTransformer::SpaceToScreenVector(const model::Point& p) const noexcept
{
    return SDL_FPoint{ static_cast<float>(
        static_cast<float>(p.x) / (m_spaceRect.w) * m_screenRect.w),
        static_cast<float>(
        static_cast<float>(p.y) / (m_spaceRect.h) * m_screenRect.h)
    };
}

node::model::Rect node::SpaceScreenTransformer::ScreenToSpaceRect(const SDL_FRect& rect) const noexcept
{
    model::Point p1 = ScreenToSpacePoint({ rect.x, rect.y });
    model::Point p2 = ScreenToSpaceVector({ rect.w, rect.h });
    return { p1.x, p1.y, p2.x, p2.y };
}

SDL_FRect node::SpaceScreenTransformer::SpaceToScreenRect(const node::model::Rect& rect) const noexcept
{
    SDL_FPoint p1 = SpaceToScreenPoint({ rect.x, rect.y });
    SDL_FPoint p2 = SpaceToScreenVector({ rect.w, rect.h });
    return { p1.x, p1.y, p2.x, p2.y };
}

SDL_FRect node::ToSDLRect(const model::Rect& rect)
{
    return { 
        static_cast<float>(rect.x), 
        static_cast<float>(rect.y), 
        static_cast<float>(rect.w), 
        static_cast<float>(rect.h) 
    };
}

SDL_FPoint node::ToSDLPoint(const model::Point& point)
{
    return { 
        static_cast<float>(point.x), 
        static_cast<float>(point.y) 
    };
}
