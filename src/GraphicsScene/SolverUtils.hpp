#pragma once

#include "GraphicsScene/NetObject.hpp"
#include "NetUtils/NetsSolver.hpp"
#include "NodeModels/SceneModelManager.hpp"
#include <span>

namespace node
{

namespace logic
{

struct NetBranch
{
	std::vector<HandlePtrS<NetNode, GraphicsObject>> nodes;
	std::vector<HandlePtrS<NetSegment, GraphicsObject>> segments;
};

NetBranch GetNetBranchForLeafNode(node::NetNode& leaf_node);

void ApplySolutionToNodes(const node::NetsSolver::NetSolution& solution, std::span<node::HandlePtrS<node::NetNode, node::GraphicsObject>> nodes,
	std::span<node::HandlePtrS<node::NetSegment, node::GraphicsObject>> segments);

struct NetNodeModificationInfo
{
	NetModificationRequest::NodeIdType node_type{};
	model::NetNodeId node_id{ 0 };
	size_t index_in_orig{};
};

struct ModificationReport
{
	NetModificationRequest request;
	NetNodeModificationInfo start_node_info;
	NetNodeModificationInfo end_node_info;
};

ModificationReport MakeModificationsReport(const node::NetsSolver::NetSolution& solution,
	std::span<const node::HandlePtrS<node::NetNode, node::GraphicsObject>> orig_nodes,
	std::span<const node::HandlePtrS<node::NetSegment, node::GraphicsObject>> orig_segments);

void UpdateModificationEndWithSocket(
	std::span<const node::HandlePtrS<node::NetNode, node::GraphicsObject>> orig_nodes,
	ModificationReport& report, const node::BlockSocketObject* end_socket);

void MergeModificationRequests(const node::NetModificationRequest& src,
	node::NetModificationRequest& target);
}
}
