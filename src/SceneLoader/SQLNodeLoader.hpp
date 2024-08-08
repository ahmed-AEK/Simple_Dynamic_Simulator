#pragma once

#include <memory>

#include "SceneLoader/NodeLoader.hpp"
#include "NodeModels/BlockModel.hpp"
#include "SQLiteCpp/Database.h"

namespace node::loader
{

class SQLNodeLoader : public NodeLoader
{
public:
	SQLNodeLoader(std::string dbname, SQLite::Database& db)
		: m_dbname{ dbname }, m_db{ db } {}
	bool AddBlock(const node::model::BlockModel& node) override;
	bool DeleteBlockAndSockets(const node::model::BlockId& node_id) override;
	bool UpdateBlockPosition(const node::model::BlockId& node_id,
		const node::model::Point& position) override;
	std::optional<node::model::BlockModel>
		GetBlock(const node::model::BlockId& block_id) override;
	bool UpdateBlockBounds(const node::model::BlockId& node_id,
		const node::model::Rect& bounds) override;

	bool AddSocket(const node::model::BlockSocketModel& socket, const model::BlockId& block_id) override;
	bool DeleteSocket(const node::model::SocketUniqueId& socket_id) override;
	bool UpdateSocketPosition(const node::model::SocketUniqueId& socket_id,
		const node::model::Point& position) override;
	node::model::BlockId GetNextBlockId() override;
	std::vector<node::model::BlockModel> GetBlocks() override;
	void LoadSocketsForBlock(node::model::BlockModel& node);

private:
	std::string m_dbname;
	SQLite::Database& m_db;

};

}