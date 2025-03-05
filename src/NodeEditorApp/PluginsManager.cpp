#include "PluginsManager.hpp"
#include "PluginAPI/BlocksPlugin.hpp"
#include "PluginAPI/BlockClassesManager.hpp"
#include "BlockPalette/PaletteProvider.hpp"

node::PluginsManager::PluginsManager(std::shared_ptr<PaletteProvider> palette, std::shared_ptr<BlockClassesManager> classes_mgr)
	:m_block_palette{std::move(palette)}, m_classes_mgr{std::move(classes_mgr)}
{
}

void node::PluginsManager::AddPlugin(std::shared_ptr<IBlocksPlugin> plugin)
{
	assert(plugin);
	if (!plugin)
	{
		SDL_Log("null plugin received!");
		return;
	}
	auto plugin_name = plugin->GetPluginName();
	if (m_plugins.find(plugin_name) != m_plugins.end())
	{
		SDL_Log("plugin already registered: %s", plugin_name.c_str());
		assert(false);
		return;
	}
	auto plugin_it = m_plugins.emplace(std::move(plugin_name), plugin);
	auto&& classes = plugin->GetClasses();
	plugin_it.first->second.registered_class_names.reserve(classes.size());
	for (const auto& cls : classes)
	{
		bool added = m_classes_mgr->RegisterBlockClass(cls);
		if (added)
		{
			plugin_it.first->second.registered_class_names.push_back(cls->GetName());
		}
		else
		{
			SDL_Log("Failed to register class: %s", cls->GetName().c_str());
		}
	}

	auto&& blocks = plugin->GetBlocks();
	plugin_it.first->second.block_ids.reserve(blocks.size());

	for (const auto& block : blocks)
	{
		auto id = m_block_palette->AddElement(block);
		if (id)
		{
			plugin_it.first->second.block_ids.push_back(*id);
		}
		else
		{
			SDL_Log("Failed to Add block to pallete: %s", block.template_name.c_str());
		}
	}
	
}

