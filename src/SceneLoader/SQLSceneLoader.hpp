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

	bool Reset() override;
	std::optional<node::model::NodeSceneModel> Load(SubSceneId id) override;
	std::optional<std::vector<SubSceneId>> GetChildSubScenes(SubSceneId id) override;
	bool Save(const node::model::NodeSceneModel& scene, SubSceneId id, SubSceneId parent_id) override;
	std::shared_ptr<BlockLoader> GetBlockLoader(SubSceneId id) override;
	std::shared_ptr<NetLoader> GetNetLoader(SubSceneId id) override;

private:
	std::string m_dbname;
	std::optional<SQLite::Database> m_db;
};


}