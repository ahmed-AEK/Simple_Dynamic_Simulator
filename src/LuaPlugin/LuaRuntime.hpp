#pragma once

#include "PluginAPI/PluginRuntime.hpp"
#include "PluginAPI/BlocksPlugin.hpp"

namespace node
{

	class LuaBasePlugin : public IBlocksPlugin
	{
	public:
		void GetPluginName(GetPluginNameCallback cb, void* context) override;

		void GetClasses(GetClassesCallback cb, void* context) override;

		void GetBlocks(GetBlocksCallback cb, void* context) override;
	private:

	};

	class LuaRuntime : public IPluginRuntime
	{
	public:
		void GetName(GetNameCallback cb, void* context) const override;
		void GetPlugin(std::string_view path, IBlocksPlugin** plugin_ptr) override;
		void GetDefaultPlugin(IBlocksPlugin** plugin_ptr) override;
	private:
	};
}