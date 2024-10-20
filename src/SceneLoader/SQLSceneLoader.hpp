#pragma once

#include "SceneLoader/SceneLoader.hpp"
#include "SQLiteCpp/Database.h"
#include "SceneLoader/SQLBlockLoader.hpp"
#include "SceneLoader/SQLNetLoader.hpp"

namespace node::loader
{

class SQLSceneLoader: public SceneLoader
{
public:
	explicit SQLSceneLoader(std::string dbname)
		: m_dbname{ dbname }, m_db{ dbname , SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE } {
		m_db.exec("PRAGMA synchronous = OFF;");
	}

	std::optional<node::model::NodeSceneModel> Load() override;
	bool Save(const node::model::NodeSceneModel& scene) override;
	std::shared_ptr<BlockLoader> GetBlockLoader() override;
	std::shared_ptr<NetLoader> GetNetLoader() override;

private:
	std::string m_dbname;
	SQLite::Database m_db;
	std::shared_ptr<SQLBlockLoader> m_nodeLoader = { 
		std::make_shared<SQLBlockLoader>(m_dbname, m_db) };
	std::shared_ptr<SQLNetLoader> m_netLoader = { 
		std::make_shared<SQLNetLoader>(m_dbname, m_db) };
};


}