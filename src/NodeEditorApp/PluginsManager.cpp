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
		m_logger.LogError("null plugin runtime received!");
		return;
	}

	std::string runtime_name = GetRuntimeName(runtime);
	if (!runtime_name.size())
	{
		m_logger.LogError("Cannot register runtime with empty name!");
		return;
	}

	auto it = m_plugin_runtimes.find(runtime_name);
	if (it != m_plugin_runtimes.end())
	{
		m_logger.LogError("attempted to add runtime again: {}", runtime_name);
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

static std::string GetClassName(node::IBlockClass& block)
{
	std::string ret;
	block.GetName([](void* context, std::string_view description) { *static_cast<std::string*>(context) = std::string{ description }; }, &ret);
	return ret;
}

template <typename Functor>
static void get_plugin_classes(node::IBlocksPlugin* plugin_ptr, Functor&& f)
	requires requires (std::span<node::IBlockClass*> classes) { f(classes); }
{
	using namespace node;
	plugin_ptr->GetClasses([](void* context, std::span<IBlockClass* const> classes) {
		Functor* functor = static_cast<Functor*>(context);
		(*functor)(classes);
		}, &f);
}

void node::PluginsManager::AddPlugin(node::BlocksPluginPtr plugin_ptr, std::string loader_name)
{
	assert(plugin_ptr);
	if (!plugin_ptr)
	{
		m_logger.LogError("null plugin received!");
		return;
	}
	auto plugin_name = GetPluginName(*plugin_ptr);
	if (m_plugins.find(plugin_name) != m_plugins.end())
	{
		m_logger.LogError("plugin already registered: {}", plugin_name);
		assert(false);
		return;
	}
	auto plugin_it = m_plugins.emplace(std::move(plugin_name), PluginRecord{ std::move(loader_name), std::move(plugin_ptr), {}, {} });
	auto* plugin = plugin_it.first->second.plugin.get();

	get_plugin_classes(plugin, [&](std::span<node::IBlockClass* const> classes)
		{
			plugin_it.first->second.registered_class_names.reserve(classes.size());
			for (const auto& cls : classes)
			{
				bool added = m_classes_mgr->RegisterBlockClass(BlockClassPtr{ BlockClassPtr::new_reference, cls });
				auto class_name = GetClassName(*cls);
				if (added)
				{
					plugin_it.first->second.registered_class_names.push_back(class_name);
				}
				else
				{
					m_logger.LogError("Failed to register class: {}", class_name);
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
						m_logger.LogError("Failed to Add block to pallete: {}", std::string_view{ block.template_name.data, block.template_name.size });
					}
				}
				catch (std::exception& e)
				{
					m_logger.LogError("Exception in GetPluginBlocks: {}", e.what());
				}
			}
		});
}

