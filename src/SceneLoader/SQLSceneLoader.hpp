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
	explicit SQLSceneLoader(std::string dbname);

	std::optional<node::model::NodeSceneModel> Load() override;
	bool Save(const node::model::NodeSceneModel& scene) override;
	std::shared_ptr<BlockLoader> GetBlockLoader() override;
	std::shared_ptr<NetLoader> GetNetLoader() override;

private:
	std::string m_dbname;
	std::optional<SQLite::Database> m_db;
};


}