#include "SQLNetLoader.hpp"
#include "toolgui/NodeMacros.h"

bool node::loader::SQLNetLoader::AddNetNode(const node::model::NetNodeModel& node)
{
	UNUSED_PARAM(node);
	return false;
}

bool node::loader::SQLNetLoader::DeleteNetNode(const node::model::id_int node_id)
{
	UNUSED_PARAM(node_id);
	return false;
}

bool node::loader::SQLNetLoader::UpdateNetNodePosition(const node::model::id_int node_id, const node::model::Point& position)
{
	UNUSED_PARAM(node_id);
	UNUSED_PARAM(position);
	return false;
}

std::optional<node::model::NetNodeModel> node::loader::SQLNetLoader::GetNetNode(const node::model::id_int node_id)
{
	UNUSED_PARAM(node_id);
	return std::optional<node::model::NetNodeModel>();
}

bool node::loader::SQLNetLoader::SetNetNodeSegmentAt(
	const node::model::id_int node_id,
	const node::model::NetNodeModel::ConnectedSegmentSide side,
	const node::model::id_int segment_id) 
{
	UNUSED_PARAM(node_id);
	UNUSED_PARAM(side);
	UNUSED_PARAM(segment_id);
	return false;
}

node::model::id_int node::loader::SQLNetLoader::GetNextNetNodeId()
{
	return node::model::id_int();
}

bool node::loader::SQLNetLoader::AddNetSegment(const node::model::NetSegmentModel& segment)
{
	UNUSED_PARAM(segment);
	return false;
}

bool node::loader::SQLNetLoader::DeleteNetSegment(const node::model::id_int id)
{
	UNUSED_PARAM(id);
	return false;
}

bool node::loader::SQLNetLoader::UpdateNetSegment(const node::model::NetSegmentModel& segment)
{
	UNUSED_PARAM(segment);
	return false;
}

std::optional<node::model::NetSegmentModel> node::loader::SQLNetLoader::GetSegment(const node::model::id_int id)
{
	UNUSED_PARAM(id);
	return std::optional<node::model::NetSegmentModel>();
}

node::model::id_int node::loader::SQLNetLoader::GetNextSegmentId()
{
	return node::model::id_int();
}

bool node::loader::SQLNetLoader::AddNet(const node::model::NetModel& net)
{
	UNUSED_PARAM(net);
	return false;
}

bool node::loader::SQLNetLoader::DeleteNet(const node::model::id_int id)
{
	UNUSED_PARAM(id);
	return false;
}

std::optional<node::model::NetModel> node::loader::SQLNetLoader::GetNet(const node::model::id_int id)
{
	UNUSED_PARAM(id);
	return std::optional<node::model::NetModel>();
}

bool node::loader::SQLNetLoader::SetNetName(const node::model::id_int id, std::string_view name)
{
	UNUSED_PARAM(id);
	UNUSED_PARAM(name);
	return false;
}

bool node::loader::SQLNetLoader::AddNodeToNet(const node::model::id_int net_id, const node::model::id_int node_id)
{
	UNUSED_PARAM(net_id);
	UNUSED_PARAM(node_id);
	return false;
}

bool node::loader::SQLNetLoader::RemoveNodeFromNet(const node::model::id_int net_id, const node::model::id_int node_id)
{
	UNUSED_PARAM(net_id);
	UNUSED_PARAM(node_id);
	return false;
}

bool node::loader::SQLNetLoader::AddSegmentToNet(const node::model::id_int net_id, const node::model::id_int segment_id)
{
	UNUSED_PARAM(net_id);
	UNUSED_PARAM(segment_id);
	return false;
}

bool node::loader::SQLNetLoader::RemoveSegmentFromNet(const node::model::id_int net_id, const node::model::id_int segment_id)
{
	UNUSED_PARAM(net_id);
	UNUSED_PARAM(segment_id);
	return false;
}

node::model::id_int node::loader::SQLNetLoader::GetNextNetId()
{
	return node::model::id_int();
}
