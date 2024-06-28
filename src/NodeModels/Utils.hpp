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

	bool operator==(const Rect& other) const noexcept
	{
		return (origin.x == other.origin.x) && (origin.y == other.origin.y) &&
			(width == other.width) && (height == other.height);
	}
};
}