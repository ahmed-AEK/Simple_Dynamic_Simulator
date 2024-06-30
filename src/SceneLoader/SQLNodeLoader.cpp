#include "SQLNodeLoader.hpp"
#include "toolgui/NodeMacros.h"

bool node::loader::SQLNodeLoader::AddNode(const node::model::NodeModelPtr& node)
{
	{
		SQLite::Statement query{ m_db, "INSERT INTO nodes VALUES (?,?,?,?,?)" };
		query.bind(1, node->GetId());
		query.bind(2, node->GetBounds().origin.x);
		query.bind(3, node->GetBounds().origin.y);
		query.bind(4, node->GetBounds().width);
		query.bind(5, node->GetBounds().height);
		query.exec();
	}
	auto&& sockets = node->GetSockets();
	if (!std::all_of(sockets.begin(), sockets.end(),
	[&](const auto& socket) { return AddSocket(socket); }))
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

std::shared_ptr<node::model::NodeModel>
node::loader::SQLNodeLoader::GetNode(node::model::id_int node_id)
{
	using namespace node::model;

	SQLite::Statement query{ m_db, "SELECT * FROM nodes" };
	if (query.executeStep())
	{
		Rect bounds{ query.getColumn(1), query.getColumn(2),
			query.getColumn(3), query.getColumn(4) };
		std::shared_ptr<NodeModel> node = 
			std::make_shared<NodeModel>(node_id, bounds);
		LoadSocketsForNode(*node);
		node->Attach(this);
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
	query.bind(1, bounds.origin.x);
	query.bind(2, bounds.origin.y);
	query.bind(3, bounds.width);
	query.bind(4, bounds.height);
	query.bind(5, node_id);
	query.exec();
	return true;
}

bool node::loader::SQLNodeLoader::AddSocket(const node::model::NodeSocketModel& socket)
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

bool node::loader::SQLNodeLoader::DeleteSocket(const node::model::NodeSocketId& socket_id)
{
	SQLite::Statement query{ m_db, "DELETE FROM sockets WHERE id = ? AND parentid = ?" };
	query.bind(1, socket_id.m_Id);
	query.bind(2, socket_id.m_nodeId);
	query.exec();
	return true;
}

bool node::loader::SQLNodeLoader::UpdateSocketPosition(const node::model::NodeSocketId& socket_id,
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

std::vector<std::shared_ptr<node::model::NodeModel>> node::loader::SQLNodeLoader::GetNodes()
{
	using namespace node::model;

	std::vector<std::shared_ptr<NodeModel>> nodes;
	SQLite::Statement query{ m_db, "SELECT * FROM nodes" };
	while (query.executeStep())
	{
		id_int node_id = query.getColumn(0);
		Rect bounds{ query.getColumn(1), query.getColumn(2),
			query.getColumn(3), query.getColumn(4) };
		std::shared_ptr<NodeModel> node = 
			std::make_shared<NodeModel>(node_id, bounds);
		LoadSocketsForNode(*node);
		node->Attach(this);
		nodes.push_back(std::move(node));
	}
	return nodes;
}

void
node::loader::SQLNodeLoader::LoadSocketsForNode(node::model::NodeModel& node)
{
	using namespace node::model;
	SQLite::Statement querySocket{ m_db, "SELECT * FROM sockets WHERE parentid = ?" };
	querySocket.bind(1, node.GetId());
	while (querySocket.executeStep())
	{
		id_int socket_id = querySocket.getColumn(0);

		Point socketOrigin{ querySocket.getColumn(2), querySocket.getColumn(3) };
		NodeSocketModel::SocketType type =
			static_cast<node::model::NodeSocketModel::SocketType>(
				static_cast<int>(querySocket.getColumn(5)));
		node.AddSocket(node::model::NodeSocketModel{ type,{socket_id,node.GetId()}, socketOrigin });
	}
}

void node::loader::SQLNodeLoader::OnEvent(node::model::NodeEventArg& ev)
{
	switch (ev.event)
	{
	case node::model::NodeEvent::PositionChanged:
		UpdateNodePosition(ev.object.GetId(), ev.object.GetPosition());
		break;
	case node::model::NodeEvent::BoundsChanged:
		UpdateNodeBounds(ev.object.GetId(), ev.object.GetBounds());
		break;
	case node::model::NodeEvent::SocketsRepositioned:
	{
		for (auto&& socket : ev.object.GetSockets())
		{
			UpdateSocketPosition(socket.GetId(), socket.GetPosition());
		}
		break;
	}
	}
}
