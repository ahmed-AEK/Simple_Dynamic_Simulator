#pragma once

#include "BlockPalette/PaletteProvider.hpp"

namespace node
{

class IBlocksPlugin;
class PaletteProvider;
class BlockClassesManager;

class PluginsManager
{
public:
	PluginsManager(std::shared_ptr<PaletteProvider> palette, std::shared_ptr<BlockClassesManager> classes_mgr);
	void AddPlugin(std::shared_ptr<IBlocksPlugin> plugin);
private:
	std::shared_ptr<PaletteProvider> m_block_palette;
	std::shared_ptr<BlockClassesManager> m_classes_mgr;
	
	struct PluginRecord
	{
		std::shared_ptr<IBlocksPlugin> plugin;
		std::vector<PaletteProvider::ElementUniqueId> block_ids;
		std::vector<std::string> registered_class_names;
	};
	std::unordered_map<std::string, PluginRecord> m_plugins;
};

}