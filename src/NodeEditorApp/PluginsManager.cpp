#include "PluginsManager.hpp"
#include "PluginAPI/BlocksPlugin.hpp"
#include "PluginAPI/BlockClassesManager.hpp"
#include "BlockPalette/PaletteProvider.hpp"
#include "PluginAPI/BlockClassHelpers.hpp"

node::PluginsManager::PluginsManager(std::shared_ptr<PaletteProvider> palette, std::shared_ptr<BlockClassesManager> classes_mgr)
	:m_block_palette{std::move(palette)}, m_classes_mgr{std::move(classes_mgr)}
{
}

static std::string GetRuntimeName(node::PluginRuntimePtr& runtime)
{
	std::string runtime_name;

	runtime->GetName([](void* context, std::string_view s) { *static_cast<std::string*>(context) = std::string{ s }; }, &runtime_name);
	return runtime_name;
}

void node::PluginsManager::AddRuntime(node::PluginRuntimePtr runtime)
{
	assert(runtime);
	if (!runtime)
	{
		SDL_Log("null plugin runtime received!");
		return;
	}

	std::string runtime_name = GetRuntimeName(runtime);
	if (!runtime_name.size())
	{
		SDL_Log("Cannot register runtime with empty name!");
		return;
	}

	auto it = m_plugin_runtimes.find(runtime_name);
	if (it != m_plugin_runtimes.end())
	{
		SDL_Log("attempted to add runtime again: %s", runtime_name.c_str());
		return;
	}

	auto result = m_plugin_runtimes.emplace(std::move(runtime_name), std::move(runtime));

	it = result.first;

	IBlocksPlugin* block_plugin_raw = nullptr;
	it->second->GetDefaultPlugin(&block_plugin_raw);
	if (block_plugin_raw)
	{
		BlocksPluginPtr plugin{ block_plugin_raw };
		AddPlugin(std::move(plugin), it->first);
	}
}


template <typename F>
static void GetPluginBlocks(node::IBlocksPlugin& plugin, F&& functor)
	requires requires (std::span<const CBlockTemplate> s) { functor(s); }
{
	using namespace node;
	
	plugin.GetBlocks([](void* context, std::span<const CBlockTemplate> blocks_span)
		{
			auto& functor = *static_cast<F*>(context);
			functor(blocks_span);
		}
	, &functor);
}

static std::string GetPluginName(node::IBlocksPlugin& plugin)
{
	std::string runtime_name;

	plugin.GetPluginName([](void* context, std::string_view s) { *static_cast<std::string*>(context) = std::string{ s }; }, &runtime_name);
	return runtime_name;
}

template <typename Functor>
static void get_plugin_classes(node::IBlocksPlugin* plugin_ptr, Functor&& f)
	requires requires (std::span<node::IBlockClass*> classes) { f(classes); }
{
	using namespace node;
	plugin_ptr->GetClasses([](void* context, std::span<IBlockClass*> classes) {
		Functor* functor = static_cast<Functor*>(context);
		(*functor)(classes);
		}, &f);
}

void node::PluginsManager::AddPlugin(node::BlocksPluginPtr plugin_ptr, std::string loader_name)
{
	assert(plugin_ptr);
	if (!plugin_ptr)
	{
		SDL_Log("null plugin received!");
		return;
	}
	auto plugin_name = GetPluginName(*plugin_ptr);
	if (m_plugins.find(plugin_name) != m_plugins.end())
	{
		SDL_Log("plugin already registered: %s", plugin_name.c_str());
		assert(false);
		return;
	}
	auto plugin_it = m_plugins.emplace(std::move(plugin_name), PluginRecord{ std::move(loader_name), std::move(plugin_ptr), {}, {} });
	auto* plugin = plugin_it.first->second.plugin.get();

	get_plugin_classes(plugin, [&](std::span<node::IBlockClass*> classes)
		{
			plugin_it.first->second.registered_class_names.reserve(classes.size());
			for (const auto& cls : classes)
			{
				bool added = m_classes_mgr->RegisterBlockClass(BlockClassPtr{ BlockClassPtr::new_reference, cls });
				if (added)
				{
					plugin_it.first->second.registered_class_names.push_back(std::string{ cls->GetName() });
				}
				else
				{
					SDL_Log("Failed to register class: %s", cls->GetName().data());
				}
			}
		});
	

	GetPluginBlocks(*plugin, [&](std::span<const CBlockTemplate> blocks_span)
		{
			for (const auto& block : blocks_span)
			{
				try
				{
					auto id = m_block_palette->AddElement(helper::CBlockTemplateToCPP(block));
					if (id)
					{

							plugin_it.first->second.block_ids.push_back(*id);
					
					}
					else
					{
						SDL_Log("Failed to Add block to pallete: %s", std::string{ block.template_name.data, block.template_name.size }.c_str());
					}
				}
				catch (std::exception& e)
				{
					SDL_Log("Exception in GetPluginBlocks: %s", e.what());
				}
			}
		});
}

