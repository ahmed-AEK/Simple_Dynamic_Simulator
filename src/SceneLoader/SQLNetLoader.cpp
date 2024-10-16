#include "SQLNetLoader.hpp"
#include "toolgui/NodeMacros.h"

bool node::loader::SQLNetLoader::AddNetNode(const node::model::NetNodeModel& node)
{
	SQLite::Statement query{ m_db, "INSERT INTO NetNodes VALUES (?,?,?,?,?,?,?)" };
	query.bind(1, node.GetId().value);
	auto&& position = node.GetPosition();
	query.bind(2, position.x);
	query.bind(3, position.y);
	if (auto segment_id = node.GetSegmentAt(model::ConnectedSegmentSide::north))
	{
		query.bind(4, segment_id->value);
	}
	if (auto segment_id = node.GetSegmentAt(model::ConnectedSegmentSide::south))
	{
		query.bind(5, segment_id->value);
	}
	if (auto segment_id = node.GetSegmentAt(model::ConnectedSegmentSide::west))
	{
		query.bind(6, segment_id->value);
	}
	if (auto segment_id = node.GetSegmentAt(model::ConnectedSegmentSide::east))
	{
		query.bind(7, segment_id->value);
	}
	query.exec();
	return true;
}

std::vector<node::model::NetNodeModel> node::loader::SQLNetLoader::GetNetNodes()
{
	std::vector<node::model::NetNodeModel> net_nodes;
	SQLite::Statement query{ m_db, "SELECT * FROM NetNodes" };
	while (query.executeStep())
	{
		if (auto node_model = GetNode_internal(query))
		{
			net_nodes.push_back(std::move(*node_model));
		}
	}
	return net_nodes;
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
	const node::model::ConnectedSegmentSide side,
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
	SQLite::Statement query{ m_db, "INSERT INTO NetSegments VALUES (?,?,?,?)" };
	query.bind(1, segment.GetId().value);
	query.bind(2, segment.m_firstNodeId.value);
	query.bind(3, segment.m_secondNodeId.value);
	query.bind(4, static_cast<int>(segment.m_orientation));
	query.exec();
	return false;
}

std::vector<node::model::NetSegmentModel> node::loader::SQLNetLoader::GetNetSegments()
{
	std::vector<node::model::NetSegmentModel> segments;
	SQLite::Statement query{ m_db, "SELECT * FROM NetSegments" };
	while (query.executeStep())
	{
		if (auto segment_model = GetSegment_internal(query))
		{
			segments.push_back(std::move(*segment_model));
		}
	}
	return segments;
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

bool node::loader::SQLNetLoader::AddSocketNodeConnection(const node::model::SocketNodeConnection& model_connection)
{
	SQLite::Statement query{ m_db, "INSERT INTO SocketNodeConnections VALUES (?,?,?)" };
	query.bind(1, model_connection.socketId.socket_id.value);
	query.bind(2, model_connection.socketId.block_id.value);
	query.bind(3, model_connection.NodeId.value);
	query.exec();
	return true;
}

std::vector<node::model::SocketNodeConnection> node::loader::SQLNetLoader::GetSocketNodeConnections()
{
	std::vector<model::SocketNodeConnection> connections;
	SQLite::Statement query{ m_db, "SELECT * FROM SocketNodeConnections" };
	while (query.executeStep())
	{
		if (auto connection_model = GetConnection_internal(query))
		{
			connections.push_back(std::move(*connection_model));
		}
	}
	return connections;
}

std::optional<node::model::NetNodeModel> node::loader::SQLNetLoader::GetNode_internal(SQLite::Statement& query)
{
	using namespace node::model;

	NetNodeId node_id{ query.getColumn(0) };
	Point position{ query.getColumn(1), query.getColumn(2) };
	NetNodeModel node_model{ node_id, position };
	auto north_column = query.getColumn(3);
	{
		if (!north_column.isNull())
		{
			NetSegmentId segment{ north_column };
			node_model.SetSegmentAt(ConnectedSegmentSide::north, segment);
		}
	}
	{
		auto south_column = query.getColumn(4);
		if (!south_column.isNull())
		{
			NetSegmentId segment{ south_column };
			node_model.SetSegmentAt(ConnectedSegmentSide::south, segment);
		}
	}
	{
		auto west_column = query.getColumn(5);
		if (!west_column.isNull())
		{
			NetSegmentId segment{ west_column };
			node_model.SetSegmentAt(ConnectedSegmentSide::west, segment);
		}
	}
	{
		auto east_column = query.getColumn(6);
		if (!east_column.isNull())
		{
			NetSegmentId segment{ east_column };
			node_model.SetSegmentAt(ConnectedSegmentSide::east, segment);
		}
	}
	return node_model;
}

std::optional<node::model::NetSegmentModel> node::loader::SQLNetLoader::GetSegment_internal(SQLite::Statement& query)
{
	using namespace node::model;
	NetSegmentId id{ query.getColumn(0) };
	NetNodeId first_segment{ query.getColumn(1) };
	NetNodeId second_segment{ query.getColumn(2) };
	NetSegmentOrientation orientation{ static_cast<int>(query.getColumn(3)) };

	return NetSegmentModel{ id, first_segment, second_segment, orientation };
}

std::optional<node::model::SocketNodeConnection> node::loader::SQLNetLoader::GetConnection_internal(SQLite::Statement& query)
{
	using namespace node::model;
	SocketUniqueId socket_id{ SocketId{query.getColumn(0)}, BlockId{query.getColumn(1)} };
	NetNodeId node_id{ query.getColumn(2) };
	return std::optional<node::model::SocketNodeConnection>{std::in_place, socket_id, node_id};
}
