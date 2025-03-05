#pragma once

#include "PluginAPI/BlockClass.hpp"

namespace node
{

class IBlocksPlugin
{
public:
	virtual std::string GetPluginName() = 0;

	virtual std::vector<std::shared_ptr<BlockClass>> GetClasses() = 0;

	virtual std::vector<BlockTemplate> GetBlocks() = 0;
};

class BlocksPlugin : IBlocksPlugin
{
public:
	std::string GetPluginName() override
	{
		return m_name;
	}

	std::vector<std::shared_ptr<BlockClass>> GetClasses()
	{
		return m_classes;
	}

	std::vector<BlockTemplate> GetBlocks() override
	{
		return m_blocks;
	}

	void AddClass(std::shared_ptr<BlockClass> cls)
	{
		m_classes.push_back(cls);
	}

	void AddBlock(const BlockTemplate& block)
	{
		m_blocks.emplace_back(block);
	}

	std::vector<std::shared_ptr<BlockClass>> m_classes;
	std::vector<BlockTemplate> m_blocks;
	std::string m_name;
};

}