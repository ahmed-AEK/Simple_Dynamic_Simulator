#pragma once

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/NetModel.hpp"
#include <memory>

namespace node::model
{

class NodeSceneModel
{
public:

	auto GetBlocks() const { return std::span{ m_blocks }; }
	auto GetBlocks() { return std::span{ m_blocks }; }

	void AddBlock(BlockModel&& block)
	{ m_blocks.push_back(std::move(block)); }

	std::optional<node::model::BlockModelRef> GetBlockById(const BlockId& id);
	void RemoveBlockById(const BlockId& id);

	auto GetNets() const { return std::span{ m_nets }; }
	auto GetNets() { return std::span{ m_nets }; }

	void AddNet(NetModel&& net) { m_nets.push_back(std::move(net)); }

	std::optional<NetModelRef>
		GetNetById(NetId id);
	void RemoveNetById(NetId id);

	void ReserveBlocks(size_t size) { m_blocks.reserve(size); }
	void ReserveNets(size_t size) { m_nets.reserve(size); }
private:
	std::vector<BlockModel> m_blocks;
	std::vector<NetModel> m_nets;
};


}