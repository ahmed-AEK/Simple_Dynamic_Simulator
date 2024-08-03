#include "SQLNodeLoader.hpp"
#include "toolgui/NodeMacros.h"
#include <algorithm>

bool node::loader::SQLNodeLoader::AddBlock(const node::model::BlockModelPtr& node)
{
	{
		SQLite::Statement query{ m_db, "INSERT INTO blocks VALUES (?,?,?,?,?)" };
		query.bind(1, node->GetId().value);
		query.bind(2, node->GetBounds().x);
		query.bind(3, node->GetBounds().y);
		query.bind(4, node->GetBounds().w);
		query.bind(5, node->GetBounds().h);
		query.exec();
	}

	auto&& sockets = node->GetSockets();
	if (!std::all_of(sockets.begin(), sockets.end(),
		[&](const auto& socket) { return AddSocket(socket, node->GetId()); }))
	{
		return false;
	}
	return true;
}

bool node::loader::SQLNodeLoader::DeleteBlockAndSockets(const node::model::BlockId& node_id)
{
	{
		SQLite::Statement query{ m_db, "DELETE FROM sockets WHERE parentid = ?" };
		query.bind(1, node_id.value);
		query.exec();
	}
	SQLite::Statement query{ m_db, "DELETE FROM blocks WHERE id = ?" };
	query.bind(1, node_id.value);
	query.exec();
	return true;
}

node::model::BlockModelPtr 
node::loader::SQLNodeLoader::GetBlock(const node::model::BlockId& block_id)
{
	using namespace node::model;

	SQLite::Statement query{ m_db, "SELECT * FROM blocks" };
	if (query.executeStep())
	{
		Rect bounds{ query.getColumn(1), query.getColumn(2),
			query.getColumn(3), query.getColumn(4) };
		std::shared_ptr<BlockModel> node = 
			std::make_shared<BlockModel>(block_id, bounds);
		LoadSocketsForBlock(*node);
		return node;
	}
	return {};
}


bool node::loader::SQLNodeLoader::UpdateBlockPosition(const node::model::BlockId& node_id,
	const node::model::Point& position)
{
	SQLite::Statement query{ m_db, "UPDATE blocks SET x = ?, y = ? WHERE id = ?" };
	query.bind(1, position.x);
	query.bind(2, position.y);
	query.bind(3, node_id.value);
	query.exec();
	return true;
}

bool node::loader::SQLNodeLoader::UpdateBlockBounds(const node::model::BlockId& node_id,
	const node::model::Rect& bounds)
{
	SQLite::Statement query{ m_db, "UPDATE blocks SET x = ?, y = ?, w = ?, h = ? WHERE id = ?" };
	query.bind(1, bounds.x);
	query.bind(2, bounds.y);
	query.bind(3, bounds.w);
	query.bind(4, bounds.h);
	query.bind(5, node_id.value);
	query.exec();
	return true;
}

bool node::loader::SQLNodeLoader::AddSocket(const node::model::BlockSocketModel& socket, 
	const node::model::BlockId& block_id)
{
	SQLite::Statement querySocket{ m_db, "INSERT INTO sockets VALUES (?,?,?,?,?)" };
	querySocket.bind(1, socket.GetId().value);
	querySocket.bind(2, block_id.value);
	querySocket.bind(3, socket.GetPosition().x);
	querySocket.bind(4, socket.GetPosition().y);
	querySocket.bind(5, static_cast<int>(socket.GetType()));
	if (auto val = socket.GetConnectedNetNode(); val)
	{
		querySocket.bind(6, (*val).value);
	}
	else
	{
		querySocket.bind(6);
	}
	querySocket.exec();
	return true;
}

bool node::loader::SQLNodeLoader::DeleteSocket(const node::model::SocketUniqueId& socket_id)
{
	SQLite::Statement query{ m_db, "DELETE FROM sockets WHERE id = ? AND parentid = ?" };
	query.bind(1, socket_id.socket_id.value);
	query.bind(2, socket_id.block_id.value);
	query.exec();
	return true;
}

bool node::loader::SQLNodeLoader::UpdateSocketPosition(const node::model::SocketUniqueId& socket_id,
	const node::model::Point& position)
{
	SQLite::Statement query{ m_db, "UPDATE sockets SET x = ?, y = ? WHERE id = ? AND parentid = ?" };
	query.bind(1, position.x);
	query.bind(2, position.y);
	query.bind(3, socket_id.socket_id.value);
	query.bind(4, socket_id.block_id.value);
	query.exec();
	return true;
}

node::model::BlockId node::loader::SQLNodeLoader::GetNextBlockId()
{
	SQLite::Statement query{ m_db, "SELECT MAX(id) FROM blocks" };
	auto result = query.executeStep();
	UNUSED_PARAM(result);
	assert(result);
	return model::BlockId{ static_cast<node::model::id_int>(query.getColumn(0)) + 1 };
}

std::vector<std::shared_ptr<node::model::BlockModel>> node::loader::SQLNodeLoader::GetBlocks()
{
	using namespace node::model;

	std::vector<std::shared_ptr<BlockModel>> nodes;
	SQLite::Statement query{ m_db, "SELECT * FROM blocks" };
	while (query.executeStep())
	{
		BlockId block_id{ query.getColumn(0) };
		Rect bounds{ query.getColumn(1), query.getColumn(2),
			query.getColumn(3), query.getColumn(4) };
		std::shared_ptr<BlockModel> node =
			std::make_shared<BlockModel>(block_id, bounds);
		LoadSocketsForBlock(*node);
		nodes.push_back(std::move(node));
	}
	return nodes;
}

void
node::loader::SQLNodeLoader::LoadSocketsForBlock(node::model::BlockModel& node)
{
	using namespace node::model;
	SQLite::Statement querySocket{ m_db, "SELECT * FROM sockets WHERE parentid = ?" };
	querySocket.bind(1, node.GetId().value);
	while (querySocket.executeStep())
	{
		id_int socket_id = querySocket.getColumn(0);

		Point socketOrigin{ querySocket.getColumn(2), querySocket.getColumn(3) };
		BlockSocketModel::SocketType type =
			static_cast<node::model::BlockSocketModel::SocketType>(
				static_cast<int>(querySocket.getColumn(5)));
		node.AddSocket(node::model::BlockSocketModel{ type, model::SocketId{socket_id}, socketOrigin });
	}
}
