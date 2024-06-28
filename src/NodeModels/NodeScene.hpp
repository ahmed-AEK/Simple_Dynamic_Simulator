#pragma once

#include "NodeModels/NodeModel.hpp"
#include "NodeModels/NetModel.hpp"

namespace node::model
{

class NodeSceneModel
{
public:

	std::span<NodeModel> GetNodes() { return m_nodes; }

	void AddNode(NodeModel node) { m_nodes.push_back(std::move(node)); }

	std::optional<std::reference_wrapper<NodeModel>> 
		GetNodeById(const id_int id);
	void RemoveNodeById(id_int id);

	std::span<NetModel> getNets() { return m_nets; }

	void AddNet(NetModel net) { m_nets.push_back(std::move(net)); }

	std::optional<std::reference_wrapper<NetModel>>
		GetNetById(id_int id);
	void RemoveNetById(id_int id);

	void ReserveNodes(size_t size) { m_nodes.reserve(size); }
	void ReserveNets(size_t size) { m_nets.reserve(size); }
private:
	std::vector<NodeModel> m_nodes;
	std::vector<NetModel> m_nets;
};


}