#pragma once

#include "NodeModels/NetModel.hpp"

namespace node::loader
{


class NetLoader
{
public:
	// NetNode Functions
	virtual bool AddNetNode(const node::model::NetNodeModel& node) = 0;
	virtual std::vector<node::model::NetNodeModel> GetNetNodes() = 0;
	virtual bool DeleteNetNode(const node::model::id_int node_id) = 0;
	virtual bool UpdateNetNodePosition(
		const node::model::id_int node_id, 
		const node::model::Point& position) = 0;
	virtual	std::optional<node::model::NetNodeModel> GetNetNode(
		const node::model::id_int node_id) = 0;
	virtual bool SetNetNodeSegmentAt(
		const node::model::id_int node_id,
		const node::model::ConnectedSegmentSide side, 
		const node::model::id_int segment_id) = 0;
	virtual node::model::id_int GetNextNetNodeId() = 0;

	// NetSegment Functions
	virtual bool AddNetSegment(
		const node::model::NetSegmentModel& segment) = 0;
	virtual std::vector<node::model::NetSegmentModel> GetNetSegments() = 0;
	virtual bool DeleteNetSegment(const node::model::id_int id) = 0;
	virtual bool UpdateNetSegment(
		const node::model::NetSegmentModel& segment) = 0;
	virtual std::optional<node::model::NetSegmentModel> GetSegment(
		const node::model::id_int id) = 0;
	virtual node::model::id_int GetNextSegmentId() = 0;

	virtual bool AddSocketNodeConnection(const node::model::SocketNodeConnection& model_connection) = 0;
	virtual std::vector<model::SocketNodeConnection> GetSocketNodeConnections() = 0;

	// Nets Functions
	virtual bool AddNet(const node::model::NetModel& net) = 0;
	virtual std::vector<node::model::NetModel> GetNets() = 0;

	// virtual destructor
	virtual ~NetLoader() = default;
	NetLoader() = default;
	NetLoader(NetLoader&&) = default;
	NetLoader(const NetLoader&) = default;
	NetLoader& operator=(NetLoader&&) = default;
	NetLoader& operator=(const NetLoader&) = default;
};

}