#pragma once

#include <memory>

#include "SceneLoader/NodeLoader.hpp"
#include "NodeModels/NodeModel.hpp"
#include "SQLiteCpp/Database.h"

namespace node::loader
{

class SQLNodeLoader : public NodeLoader
{
public:
	SQLNodeLoader(std::string dbname, SQLite::Database& db)
		: m_dbname{ dbname }, m_db{ db } {}
	bool AddNode(const node::model::NodeModelPtr& node) override;
	bool DeleteNodeAndSockets(const node::model::id_int node_id) override;
	bool UpdateNodePosition(node::model::id_int node_id,
		const node::model::Point& position) override;
	std::shared_ptr<node::model::NodeModel>
		GetNode(node::model::id_int node_id) override;
	bool UpdateNodeBounds(node::model::id_int node_id,
		const node::model::Rect& bounds) override;

	bool AddSocket(const node::model::NodeSocketModel& socket) override;
	bool DeleteSocket(const node::model::NodeSocketId& socket_id) override;
	bool UpdateSocketPosition(const node::model::NodeSocketId& socket_id,
		const node::model::Point& position) override;
	node::model::id_int GetNextNodeIdx() override;
	std::vector<std::shared_ptr<node::model::NodeModel>> GetNodes() override;
	void LoadSocketsForNode(node::model::NodeModel& node);
private:
	std::string m_dbname;
	SQLite::Database& m_db;

};

}