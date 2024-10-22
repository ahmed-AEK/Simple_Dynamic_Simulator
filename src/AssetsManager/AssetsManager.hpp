#pragma once

#include <optional>
#include <span>
#include <string_view>

namespace node
{

class AssetsManager
{
public:
	std::optional<std::span<const unsigned char>> GetResource(std::string_view asset_path);
private:
};
}
