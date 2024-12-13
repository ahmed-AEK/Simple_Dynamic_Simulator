#pragma once

#include <memory>

#include "SceneLoader/BlockLoader.hpp"
#include "NodeModels/BlockModel.hpp"
#include "SQLiteCpp/Database.h"

namespace node::loader
{

class SQLBlockLoader : public BlockLoader
{
public:
	SQLBlockLoader(std::string dbname, SQLite::Database& db, SubSceneId scene_id)
		: m_dbname{ dbname }, m_db{ db }, m_scene_id{scene_id} {}
	bool AddBlock(const node::model::BlockModel& block) override;
	bool DeleteBlockAndSockets(const node::model::BlockId& block_id) override;
	bool UpdateBlockPosition(const node::model::BlockId& block_id,
		const node::model::Point& position) override;
	std::optional<node::model::BlockModel>
		GetBlock(const node::model::BlockId& block_id) override;
	bool UpdateBlockBounds(const node::model::BlockId& block_id,
		const node::model::Rect& bounds) override;

	bool AddStylerProperty(const node::model::BlockId& block_id, const std::string& name, const std::string& value, int32_t property_id) override;
	bool AddProperty(const node::model::BlockId& block_id, const model::BlockProperty& property, int32_t property_id) override;
	bool AddSocket(const node::model::BlockSocketModel& socket, const model::BlockId& block_id) override;
	bool DeleteSocket(const node::model::SocketUniqueId& socket_id) override;
	bool UpdateSocketPosition(const node::model::SocketUniqueId& socket_id,
		const node::model::Point& position) override;
	node::model::BlockId GetNextBlockId() override;
	std::vector<node::model::BlockModel> GetBlocks() override;
	void LoadSocketsForBlock(node::model::BlockModel& block);
	bool LoadPropertiesForBlock(node::model::BlockModel& block);
	bool LoadStylerProperties(node::model::BlockModel& block);
private:
	std::optional<node::model::BlockModel> GetBlock_internal(SQLite::Statement& query);
	std::string m_dbname;
	SQLite::Database& m_db;
	SubSceneId m_scene_id;
};

}