#include "SQLNodeLoader.hpp"
#include "toolgui/NodeMacros.h"

bool node::loader::SQLNodeLoader::AddNode(const node::model::BlockModelPtr& node)
{
	{
		SQLite::Statement query{ m_db, "INSERT INTO nodes VALUES (?,?,?,?,?)" };
		query.bind(1, node->GetId());
		query.bind(2, node->GetBounds().x);
		query.bind(3, node->GetBounds().y);
		query.bind(4, node->GetBounds().w);
		query.bind(5, node->GetBounds().h);
		query.exec();
	}
	auto add_sockets = [&](const node::model::BlockSocketModel::SocketType type)
		{
			auto&& sockets = node->GetSockets(type);
			if (!std::all_of(sockets.begin(), sockets.end(),
				[&](const auto& socket) { return AddSocket(socket); }))
			{
				return false;
			}
			return true;
		};
	if (!add_sockets(node::model::BlockSocketModel::SocketType::input))
	{
		return false;
	}
	if (!add_sockets(node::model::BlockSocketModel::SocketType::output))
	{
		return false;
	}	
	return true;
}

bool node::loader::SQLNodeLoader::DeleteNodeAndSockets(const node::model::id_int node_id)
{
	{
		SQLite::Statement query{ m_db, "DELETE FROM sockets WHERE parentid = ?" };
		query.bind(1, node_id);
		query.exec();
	}
	SQLite::Statement query{ m_db, "DELETE FROM nodes WHERE id = ?" };
	query.bind(1, node_id);
	query.exec();
	return true;
}

node::model::BlockModelPtr 
node::loader::SQLNodeLoader::GetNode(node::model::id_int node_id)
{
	using namespace node::model;

	SQLite::Statement query{ m_db, "SELECT * FROM nodes" };
	if (query.executeStep())
	{
		Rect bounds{ query.getColumn(1), query.getColumn(2),
			query.getColumn(3), query.getColumn(4) };
		std::shared_ptr<BlockModel> node = 
			std::make_shared<BlockModel>(node_id, bounds);
		LoadSocketsForNode(*node);
		return node;
	}
	return {};
}


bool node::loader::SQLNodeLoader::UpdateNodePosition(node::model::id_int node_id,
	const node::model::Point& position)
{
	SQLite::Statement query{ m_db, "UPDATE nodes SET x = ?, y = ? WHERE id = ?" };
	query.bind(1, position.x);
	query.bind(2, position.y);
	query.bind(3, node_id);
	query.exec();
	return true;
}

bool node::loader::SQLNodeLoader::UpdateNodeBounds(node::model::id_int node_id,
	const node::model::Rect& bounds)
{
	SQLite::Statement query{ m_db, "UPDATE nodes SET x = ?, y = ?, w = ?, h = ? WHERE id = ?" };
	query.bind(1, bounds.x);
	query.bind(2, bounds.y);
	query.bind(3, bounds.w);
	query.bind(4, bounds.h);
	query.bind(5, node_id);
	query.exec();
	return true;
}

bool node::loader::SQLNodeLoader::AddSocket(const node::model::BlockSocketModel& socket)
{
	SQLite::Statement querySocket{ m_db, "INSERT INTO sockets VALUES (?,?,?,?,?)" };
	querySocket.bind(1, socket.GetId().m_Id);
	querySocket.bind(2, socket.GetId().m_nodeId);
	querySocket.bind(3, socket.GetPosition().x);
	querySocket.bind(4, socket.GetPosition().y);
	querySocket.bind(5, static_cast<int>(socket.GetType()));
	if (auto val = socket.GetConnectedNetNode(); val)
	{
		querySocket.bind(6, *val);
	}
	else
	{
		querySocket.bind(6);
	}
	querySocket.exec();
	return true;
}

bool node::loader::SQLNodeLoader::DeleteSocket(const node::model::BlockSocketId& socket_id)
{
	SQLite::Statement query{ m_db, "DELETE FROM sockets WHERE id = ? AND parentid = ?" };
	query.bind(1, socket_id.m_Id);
	query.bind(2, socket_id.m_nodeId);
	query.exec();
	return true;
}

bool node::loader::SQLNodeLoader::UpdateSocketPosition(const node::model::BlockSocketId& socket_id,
	const node::model::Point& position)
{
	SQLite::Statement query{ m_db, "UPDATE sockets SET x = ?, y = ? WHERE id = ? AND parentid = ?" };
	query.bind(1, position.x);
	query.bind(2, position.y);
	query.bind(3, socket_id.m_Id);
	query.bind(4, socket_id.m_nodeId);
	query.exec();
	return true;
}

node::model::id_int node::loader::SQLNodeLoader::GetNextNodeIdx()
{
	SQLite::Statement query{ m_db, "SELECT MAX(id) FROM nodes" };
	auto result = query.executeStep();
	UNUSED_PARAM(result);
	assert(result);
	return static_cast<node::model::id_int>(query.getColumn(0)) + 1;
}

std::vector<std::shared_ptr<node::model::BlockModel>> node::loader::SQLNodeLoader::GetNodes()
{
	using namespace node::model;

	std::vector<std::shared_ptr<BlockModel>> nodes;
	SQLite::Statement query{ m_db, "SELECT * FROM nodes" };
	while (query.executeStep())
	{
		id_int node_id = query.getColumn(0);
		Rect bounds{ query.getColumn(1), query.getColumn(2),
			query.getColumn(3), query.getColumn(4) };
		std::shared_ptr<BlockModel> node =
			std::make_shared<BlockModel>(node_id, bounds);
		LoadSocketsForNode(*node);
		nodes.push_back(std::move(node));
	}
	return nodes;
}

void
node::loader::SQLNodeLoader::LoadSocketsForNode(node::model::BlockModel& node)
{
	using namespace node::model;
	SQLite::Statement querySocket{ m_db, "SELECT * FROM sockets WHERE parentid = ?" };
	querySocket.bind(1, node.GetId());
	while (querySocket.executeStep())
	{
		id_int socket_id = querySocket.getColumn(0);

		Point socketOrigin{ querySocket.getColumn(2), querySocket.getColumn(3) };
		BlockSocketModel::SocketType type =
			static_cast<node::model::BlockSocketModel::SocketType>(
				static_cast<int>(querySocket.getColumn(5)));
		node.AddSocket(node::model::BlockSocketModel{ type,{socket_id,node.GetId()}, socketOrigin });
	}
}
