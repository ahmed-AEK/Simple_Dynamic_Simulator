#pragma once

#include "SDL_Framework/SDL_headers.h"

inline constexpr SDL_FPoint operator+(const SDL_FPoint& p1, const SDL_FPoint& p2)
{
	return SDL_FPoint{ p1.x + p2.x, p1.y + p2.y };
}

inline constexpr SDL_FPoint operator-(const SDL_FPoint& p1, const SDL_FPoint& p2)
{
	return SDL_FPoint{ p1.x - p2.x, p1.y - p2.y };
}
