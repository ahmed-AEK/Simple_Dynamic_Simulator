#pragma once

#include "SceneLoader/SceneLoader.hpp"
#include "SQLiteCpp/Database.h"

namespace node::loader
{

class SQLSceneLoader: public SceneLoader
{
public:
	explicit SQLSceneLoader(std::string dbname)
		: m_dbname{ dbname }, m_db{ dbname , SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE } {}

	std::optional<node::model::NodeSceneModel> Load() override;
	bool Save(const node::model::NodeSceneModel& scene) override;
	std::unique_ptr<NodeLoader> GetNodeLoader() override;

private:
	std::string m_dbname;
	SQLite::Database m_db;
};


}