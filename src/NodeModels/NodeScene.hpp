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

	void AddBlock(BlockModelPtr block)
	{ m_blocks.push_back(std::move(block)); }

	BlockModelPtr GetBlockById(const id_int id);
	void RemoveBlockById(id_int id);

	auto GetNets() const { return std::span{ m_nets }; }
	auto GetNets() { return std::span{ m_nets }; }

	void AddNet(NetModel net) { m_nets.push_back(std::move(net)); }

	std::optional<std::reference_wrapper<NetModel>>
		GetNetById(id_int id);
	void RemoveNetById(id_int id);

	void ReserveBlocks(size_t size) { m_blocks.reserve(size); }
	void ReserveNets(size_t size) { m_nets.reserve(size); }
private:
	std::vector<BlockModelPtr> m_blocks;
	std::vector<NetModel> m_nets;
};


}