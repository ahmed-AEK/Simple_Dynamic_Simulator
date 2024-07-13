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
	bool AddNode(const node::model::BlockModelPtr& node) override;
	bool DeleteNodeAndSockets(const node::model::id_int node_id) override;
	bool UpdateNodePosition(node::model::id_int node_id,
		const node::model::Point& position) override;
	node::model::BlockModelPtr
		GetNode(node::model::id_int node_id) override;
	bool UpdateNodeBounds(node::model::id_int node_id,
		const node::model::Rect& bounds) override;

	bool AddSocket(const node::model::BlockSocketModel& socket) override;
	bool DeleteSocket(const node::model::BlockSocketId& socket_id) override;
	bool UpdateSocketPosition(const node::model::BlockSocketId& socket_id,
		const node::model::Point& position) override;
	node::model::id_int GetNextNodeIdx() override;
	std::vector<std::shared_ptr<node::model::BlockModel>> GetNodes() override;
	void LoadSocketsForNode(node::model::BlockModel& node);

private:
	std::string m_dbname;
	SQLite::Database& m_db;

};

}