#pragma once

#include <cstdint>

namespace node::model
{

using node_int = int32_t;
using id_int = int32_t;

struct Point
{
	node_int x;
	node_int y;
};

struct Rect
{
	node_int x;
	node_int y;
	node_int w;
	node_int h;
};

[[nodiscard]] inline constexpr bool operator==(const Point& p1, const Point& p2) noexcept
{
	return p1.x == p2.x && p1.y == p2.y;
}

[[nodiscard]] inline constexpr Point operator+(const Point& p1, const Point& p2) noexcept
{
	return { p1.x + p2.x , p1.y + p2.y };
}

[[nodiscard]] inline constexpr bool operator==(const Rect& r1, const Rect& r2) noexcept
{
	return (r1.x == r2.x) && (r1.y == r2.y) &&
		(r1.w == r2.w) && (r1.h == r2.h);
}

}