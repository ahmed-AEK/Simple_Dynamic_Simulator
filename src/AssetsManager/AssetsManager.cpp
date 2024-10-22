#include "AssetsManager.hpp"
#include "data/resources.hpp"

std::optional<std::span<const unsigned char>> node::AssetsManager::GetResource(std::string_view asset_path)
{
	for (auto&& item : assets_listing)
	{
		if (item.path == asset_path)
		{
			return std::span<const unsigned char>{ item.data, item.data + item.size };
		}
	}
	return std::nullopt;
}
