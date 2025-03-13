#pragma once

#include "PluginAPI/BlocksPlugin.hpp"
#include "PluginAPI/PluginRuntime.hpp"

namespace node
{

class BuiltinClassesPlugin: public IBlocksPlugin
{
public:
	BuiltinClassesPlugin();

	void GetPluginName(GetPluginNameCallback cb, void* context) override;

	std::vector<std::shared_ptr<BlockClass>> GetClasses() override;

	void GetBlocks(GetBlocksCallback cb, void* context) override;
};


class NativePluginsRuntime : public IPluginRuntime
{
public:
	virtual void GetName(GetNameCallback cb, void* context) const override;
	void GetPlugin(std::string_view path, IBlocksPlugin** plugin_ptr) override;
	void GetDefaultPlugin(IBlocksPlugin** plugin_ptr) override;
};

}