#pragma once

#include "BlockPalette/PaletteProvider.hpp"
#include "PluginAPI/BlocksPlugin.hpp"
#include "PluginAPI/PluginRuntime.hpp"

namespace node
{

class IBlocksPlugin;
class PaletteProvider;
class BlockClassesManager;

class PluginsManager
{
public:
	PluginsManager(std::shared_ptr<PaletteProvider> palette, std::shared_ptr<BlockClassesManager> classes_mgr);
	void AddRuntime(node::PluginRuntimePtr runtime);
	void AddPlugin(node::BlocksPluginPtr plugin, std::string loader_name = {});
private:
	std::shared_ptr<PaletteProvider> m_block_palette;
	std::shared_ptr<BlockClassesManager> m_classes_mgr;
	std::unordered_map<std::string, node::PluginRuntimePtr> m_plugin_runtimes;

	struct PluginRecord
	{
		std::string loader_name;
		node::BlocksPluginPtr plugin;
		std::vector<PaletteProvider::ElementUniqueId> block_ids;
		std::vector<std::string> registered_class_names;
	};
	std::unordered_map<std::string, PluginRecord> m_plugins;
	logging::Logger m_logger = logger(logging::LogCategory::Core);
};

}