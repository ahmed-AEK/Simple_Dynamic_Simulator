#pragma once

#include "NodeModels/BlockModel.hpp"
#include "NodeModels/NetModel.hpp"
#include "NodeModels/FunctionalBlocksDataManager.hpp"
#include "NodeModels/SubsystemBlocksDataManager.hpp"
#include "NodeModels/PortBlocksDataManager.hpp"
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

	node::model::BlockModel* GetBlockById(const BlockId& id);
	const node::model::BlockModel* GetBlockById(const BlockId& id) const;
	void RemoveBlockById(const BlockId& id);

	void AddNetNode(NetNodeModel&& netNode) { m_nodes.push_back(std::move(netNode)); }
	void RemoveNetNodeById(const NetNodeId& id);

	const NetNodeModel*
		GetNetNodeById(const NetNodeId& id) const;

	NetNodeModel*
		GetNetNodeById(const NetNodeId& id);

	std::span<NetNodeModel>
		GetNetNodes() { return m_nodes; }
	std::span<const NetNodeModel>
		GetNetNodes() const { return m_nodes; }

	void AddNetSegment(NetSegmentModel&& netSegment) { m_segments.push_back(std::move(netSegment)); }
	void RemoveNetSegmentById(const NetSegmentId& id);

	const NetSegmentModel*
		GetNetSegmentById(const NetSegmentId& id) const;
	NetSegmentModel*
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
	node::model::SocketNodeConnection*
		GetSocketConnectionForSocket(const model::SocketUniqueId& socket_id);
	const node::model::SocketNodeConnection*
		GetSocketConnectionForSocket(const model::SocketUniqueId& socket_id) const;
	const node::model::SocketNodeConnection*
		GetSocketConnectionForNode(const model::NetNodeId& node_id) const;
	node::model::SocketNodeConnection*
		GetSocketConnectionForNode(const model::NetNodeId& node_id);

	void ReserveBlocks(size_t size) { m_blocks.reserve(size); }
	void ReserveNetSegments(size_t size) { m_segments.reserve(size); }
	void ReserveNetNodes(size_t size) { m_nodes.reserve(size); }

	FunctionalBlocksDataManager& GetFunctionalBlocksManager() { return m_functionalBlocksManager; }
	const FunctionalBlocksDataManager& GetFunctionalBlocksManager() const { return m_functionalBlocksManager; }

	SubsystemBlocksDataManager& GetSubsystemBlocksManager() { return m_subsystemBlocksManager; }
	const SubsystemBlocksDataManager& GetSubsystemBlocksManager() const { return m_subsystemBlocksManager; }

	PortBlocksDataManager& GetPortBlocksManager() { return m_portBlocksManager; }
	const PortBlocksDataManager& GetPortBlocksManager() const { return m_portBlocksManager; }

	SubSceneId GetSubSceneId() const { return m_id; };
	void SetSubSceneId(SubSceneId id) { m_id = id; }

	void AddNet(NetId id, const NetCategory& category);
	NetModel* GetNet(NetId id);
	const NetModel* GetNet(NetId id) const;
private:
	SubSceneId m_id;
	std::vector<BlockModel> m_blocks;
	std::vector<NetNodeModel> m_nodes;
	std::vector<NetSegmentModel> m_segments;
	std::vector<SocketNodeConnection> m_SocketConnections;
	std::vector<NetModel> m_nets;
	FunctionalBlocksDataManager m_functionalBlocksManager;
	SubsystemBlocksDataManager m_subsystemBlocksManager;
	PortBlocksDataManager m_portBlocksManager;
};


}