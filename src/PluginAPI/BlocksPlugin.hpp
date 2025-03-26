#pragma once

#include "PluginAPI/BlockClass.hpp"
#include "PluginAPI/BlockClassHelpers.hpp"

namespace node
{

class IBlocksPlugin
{
public:
	using GetPluginNameCallback = void(*)(void*, std::string_view);
	virtual void GetPluginName(GetPluginNameCallback cb, void* context) = 0;

	using GetClassesCallback = void(*)(void*, std::span<IBlockClass*>);
	virtual void GetClasses(GetClassesCallback cb, void* context) = 0;

	using GetBlocksCallback = void(*)(void* context, std::span<const CBlockTemplate> blocks);

	virtual void GetBlocks(GetBlocksCallback cb, void* context) = 0;

	virtual void Destroy() { delete this; }
protected:
	virtual ~IBlocksPlugin() = default;
};

namespace detail
{
	struct BlocksPluginDeleter
	{
		void operator()(IBlocksPlugin* ptr) { if (ptr) { ptr->Destroy(); } };
	};
}

using BlocksPluginPtr = std::unique_ptr<IBlocksPlugin, detail::BlocksPluginDeleter>;

template <typename T, typename...Args>
BlocksPluginPtr make_BlocksPlugin(Args&&...args)
{
	return BlocksPluginPtr{ new T{std::forward<Args>(args)...} };
}


class BlocksPlugin : IBlocksPlugin
{
public:

	void GetPluginName(GetPluginNameCallback cb, void* context) override
	{
		cb(context, m_name);
	}

	std::vector<BlockClassPtr> GetClasses()
	{
		return m_classes;
	}

	void GetBlocks(GetBlocksCallback cb, void* context) override
	{
		auto group = helper::BlockTemplateGroupToC(m_blocks);

		cb(context, group.block_templates);
	}

	void AddClass(BlockClassPtr cls)
	{
		m_classes.push_back(cls);
	}

	void AddBlock(const BlockTemplate& block)
	{
		m_blocks.emplace_back(block);
	}

	std::vector<BlockClassPtr> m_classes;
	std::vector<BlockTemplate> m_blocks;
	std::string m_name;
};

}