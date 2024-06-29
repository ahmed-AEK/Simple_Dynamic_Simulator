#pragma once

#include "NodeModels/NetModel.hpp"

namespace node::loader
{


class NetLoader
{
public:
	// NetNode Functions
	virtual bool AddNetNode(const node::model::NetNodeModel& node) = 0;
	virtual bool DeleteNetNode(const node::model::id_int node_id) = 0;
	virtual bool UpdateNetNodePosition(
		const node::model::id_int node_id, 
		const node::model::Point& position) = 0;
	virtual	std::optional<node::model::NetNodeModel> GetNetNode(
		const node::model::id_int node_id) = 0;
	virtual bool SetNetNodeSegmentAt(
		const node::model::id_int node_id,
		const node::model::NetNodeModel::ConnectedSegmentSide side, 
		const node::model::id_int segment_id) = 0;
	virtual node::model::id_int GetNextNetNodeId() = 0;

	// NetSegment Functions
	virtual bool AddNetSegment(
		const node::model::NetSegmentModel& segment) = 0;
	virtual bool DeleteNetSegment(const node::model::id_int id) = 0;
	virtual bool UpdateNetSegment(
		const node::model::NetSegmentModel& segment) = 0;
	virtual std::optional<node::model::NetSegmentModel> GetSegment(
		const node::model::id_int id) = 0;
	virtual node::model::id_int GetNextSegmentId() = 0;

	// Net Functions
	virtual bool AddNet(const node::model::NetModel& net) = 0;
	virtual bool DeleteNet(const node::model::id_int id) = 0;
	virtual std::optional<node::model::NetModel> GetNet(
		const node::model::id_int id) = 0;
	virtual bool SetNetName(const node::model::id_int id, 
		std::string_view name) = 0;
	virtual bool AddNodeToNet(const node::model::id_int net_id,
		const node::model::id_int node_id) = 0;
	virtual bool RemoveNodeFromNet(const node::model::id_int net_id,
		const node::model::id_int node_id) = 0;
	virtual bool AddSegmentToNet(const node::model::id_int net_id,
		const node::model::id_int segment_id) = 0;
	virtual bool RemoveSegmentFromNet(const node::model::id_int net_id,
		const node::model::id_int segment_id) = 0;
	virtual node::model::id_int GetNextNetId() = 0;

	// virtual destructor
	virtual ~NetLoader() = default;
	NetLoader() = default;
	NetLoader(NetLoader&&) = default;
	NetLoader(const NetLoader&) = default;
	NetLoader& operator=(NetLoader&&) = default;
	NetLoader& operator=(const NetLoader&) = default;
};

}