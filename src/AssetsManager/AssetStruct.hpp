#pragma once

#include <cstddef>

struct AssetStruct
{
	size_t size;
	const char* path;
	const unsigned char* data;
};
