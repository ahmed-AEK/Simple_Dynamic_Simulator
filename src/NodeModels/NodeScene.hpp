#pragma once

#include "NodeModels/NodeModel.hpp"
#include "NodeModels/NetModel.hpp"
#include <memory>

namespace node::model
{

class NodeSceneModel
{
public:

	auto GetNodes() const { return std::span{ m_nodes }; }
	auto GetNodes() { return std::span{ m_nodes }; }

	void AddNode(NodeModelPtr node)
	{ m_nodes.push_back(std::move(node)); }

	std::shared_ptr<node::model::NodeModel>
		GetNodeById(const id_int id);
	void RemoveNodeById(id_int id);

	auto GetNets() const { return std::span{ m_nets }; }
	auto GetNets() { return std::span{ m_nets }; }

	void AddNet(NetModel net) { m_nets.push_back(std::move(net)); }

	std::optional<std::reference_wrapper<NetModel>>
		GetNetById(id_int id);
	void RemoveNetById(id_int id);

	void ReserveNodes(size_t size) { m_nodes.reserve(size); }
	void ReserveNets(size_t size) { m_nets.reserve(size); }
private:
	std::vector<NodeModelPtr> m_nodes;
	std::vector<NetModel> m_nets;
};


}