#pragma once

#include "PluginAPI/BlocksPlugin.hpp"

namespace node
{

class BuiltinClassesPlugin: public IBlocksPlugin
{
public:

	std::string GetPluginName() override;

	std::vector<std::shared_ptr<BlockClass>> GetClasses() override;

	std::vector<BlockTemplate> GetBlocks() override;
};

}