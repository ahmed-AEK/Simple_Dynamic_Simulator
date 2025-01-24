#include "SpaceScreenTransformer.hpp"
#include <cmath>

node::SpaceScreenTransformer::SpaceScreenTransformer(const SDL_FRect& screen_rect, const model::Rect& space_rect)
    :m_screen_orig{screen_rect.x, screen_rect.y}, m_space_orig{space_rect.x, space_rect.y}, 
    m_space_to_screen_factor_x{screen_rect.w/space_rect.w}, m_space_to_screen_factor_y{screen_rect.h/space_rect.h}
{
    if (!std::isfinite(m_space_to_screen_factor_x))
    {
        m_space_to_screen_factor_x = 1;
    }
    if (!std::isfinite(m_space_to_screen_factor_y))
    {
        m_space_to_screen_factor_y = 1;
    }
}

node::SpaceScreenTransformer::SpaceScreenTransformer(const SDL_FPoint& screen_orig, const model::Point& space_orig, float space_to_screen_factor)
    : m_screen_orig{screen_orig}, m_space_orig{space_orig}, m_space_to_screen_factor_x{space_to_screen_factor}, m_space_to_screen_factor_y{space_to_screen_factor}
{
}

node::SpaceScreenTransformer::SpaceScreenTransformer()
    :m_screen_orig{0,0}, m_space_orig{0,0}, m_space_to_screen_factor_x{1}, m_space_to_screen_factor_y{1}
{
}

node::model::Point node::SpaceScreenTransformer::ScreenToSpacePoint(const SDL_FPoint& p) const noexcept
{
    return node::model::Point{
    static_cast<int>(
    static_cast<double>(p.x - m_screen_orig.x) / m_space_to_screen_factor_x + m_space_orig.x),
    static_cast<int>(
    static_cast<double>(p.y - m_screen_orig.y) / m_space_to_screen_factor_y + m_space_orig.y)
    };
}

SDL_FPoint node::SpaceScreenTransformer::SpaceToScreenPoint(const node::model::Point& p) const noexcept
{
    return SDL_FPoint{ static_cast<float>(
    static_cast<float>(p.x - m_space_orig.x) * m_space_to_screen_factor_x + m_screen_orig.x),
    static_cast<float>(
    static_cast<float>(p.y - m_space_orig.y) * m_space_to_screen_factor_y + m_screen_orig.y)
    };
}

node::model::Point node::SpaceScreenTransformer::ScreenToSpaceVector(const SDL_FPoint& p) const noexcept
{
    return model::Point{
    static_cast<int>(
    static_cast<double>(p.x) / m_space_to_screen_factor_x),
    static_cast<int>(
    static_cast<double>(p.y) / m_space_to_screen_factor_y)
    };
}

SDL_FPoint node::SpaceScreenTransformer::SpaceToScreenVector(const model::Point& p) const noexcept
{
    return SDL_FPoint{ static_cast<float>(
        static_cast<float>(p.x) * m_space_to_screen_factor_x),
        static_cast<float>(
        static_cast<float>(p.y) * m_space_to_screen_factor_y)
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
