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

};

}