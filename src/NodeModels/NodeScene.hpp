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
	std::optional<node::model::BlockModelConstRef> GetBlockById(const BlockId& id) const;
	void RemoveBlockById(const BlockId& id);

	void AddNetNode(NetNodeModel&& netNode) { m_nodes.push_back(std::move(netNode)); }
	void RemoveNetNodeById(const NetNodeId& id);

	std::optional<std::reference_wrapper<const NetNodeModel>>
		GetNetNodeById(const NetNodeId& id) const;

	std::optional<std::reference_wrapper<NetNodeModel>>
		GetNetNodeById(const NetNodeId& id);

	std::span<NetNodeModel>
		GetNetNodes() { return m_nodes; }
	std::span<const NetNodeModel>
		GetNetNodes() const { return m_nodes; }

	void AddNetSegment(NetSegmentModel&& netSegment) { m_segments.push_back(std::move(netSegment)); }
	void RemoveNetSegmentById(const NetSegmentId& id);

	std::optional<std::reference_wrapper<const NetSegmentModel>>
		GetNetSegmentById(const NetSegmentId& id) const;
	std::optional<std::reference_wrapper<NetSegmentModel>>
		GetNetSegmentById(const NetSegmentId& id);
	std::span<NetSegmentModel>
		GetNetSegments() { return m_segments; }
	std::span<const NetSegmentModel>
		GetNetSegments() const { return m_segments; }

	std::span<model::SocketNodeConnection> GetSocketConnections() { return m_SocketConnections; }
	std::span<const model::SocketNodeConnection> GetSocketConnections() const { return m_SocketConnections; }

	void AddSocketNodeConnection(const model::SocketNodeConnection& connection) {
		m_SocketConnections.push_back(connection);
	}
	void RemoveSocketConnectionForSocket(const model::SocketUniqueId& socket);
	std::optional<std::reference_wrapper<const node::model::SocketNodeConnection>>
		GetSocketConnectionForSocket(const model::SocketUniqueId& socket_id) const;
	std::optional<std::reference_wrapper<const node::model::SocketNodeConnection>>
		GetSocketConnectionForNode(const model::NetNodeId& node_id) const;
	std::optional<std::reference_wrapper<node::model::SocketNodeConnection>>
		GetSocketConnectionForNode(const model::NetNodeId& node_id);

	void ReserveBlocks(size_t size) { m_blocks.reserve(size); }
	void ReserveNetSegments(size_t size) { m_segments.reserve(size); }
	void ReserveNetNodes(size_t size) { m_nodes.reserve(size); }
private:
	std::vector<BlockModel> m_blocks;
	std::vector<NetNodeModel> m_nodes;
	std::vector<NetSegmentModel> m_segments;
	std::vector<SocketNodeConnection> m_SocketConnections;
};


}