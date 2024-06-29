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
	Point origin;
	node_int width;
	node_int height;
};

[[nodiscard]] inline constexpr bool operator==(const Point& p1, const Point& p2) noexcept
{
	return p1.x == p2.x && p1.y == p2.y;
}

[[nodiscard]] inline constexpr bool operator==(const Rect& r1, const Rect& r2) noexcept
{
	return (r1.origin.x == r2.origin.x) && (r1.origin.y == r2.origin.y) &&
		(r1.width == r2.width) && (r1.height == r2.height);
}

}