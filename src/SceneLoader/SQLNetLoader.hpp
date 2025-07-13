#pragma once

#include "SceneLoader/NetLoader.hpp"
#include "SQLiteCpp/Database.h"

namespace node::loader
{


class SQLNetLoader: public NetLoader
{
public:
	SQLNetLoader(std::string dbname, SQLite::Database& db, SubSceneId scene_id)
		: m_dbname{ dbname }, m_db{ db }, m_scene_id{ scene_id } {}

	// NetNode Functions
	bool AddNetNode(const node::model::NetNodeModel& node) override;
	std::vector<node::model::NetNodeModel> GetNetNodes() override;
	bool DeleteNetNode(const node::model::id_int node_id) override;
	bool UpdateNetNodePosition(
		const node::model::id_int node_id,
		const node::model::Point& position) override;
	std::optional<node::model::NetNodeModel> GetNetNode(
		const node::model::id_int node_id) override;
	bool SetNetNodeSegmentAt(
		const node::model::id_int node_id,
		const node::model::ConnectedSegmentSide side,
		const node::model::id_int segment_id) override;
	node::model::id_int GetNextNetNodeId() override;

	// NetSegment Functions
	bool AddNetSegment(
		const node::model::NetSegmentModel& segment) override;
	std::vector<node::model::NetSegmentModel> GetNetSegments() override;
	bool DeleteNetSegment(const node::model::id_int id) override;
	bool UpdateNetSegment(
		const node::model::NetSegmentModel& segment) override;
	std::optional<node::model::NetSegmentModel> GetSegment(
		const node::model::id_int id) override;
	node::model::id_int GetNextSegmentId() override;

	// SocketConnection Functions
	bool AddSocketNodeConnection(const node::model::SocketNodeConnection& model_connection) override;
	std::vector<model::SocketNodeConnection> GetSocketNodeConnections() override;

	// Net Functions
	bool AddNet(const node::model::NetModel& net) override;
	std::vector<model::NetModel> GetNets() override;

private:
	std::optional<node::model::NetNodeModel> GetNode_internal(SQLite::Statement& query);
	std::optional<node::model::NetSegmentModel> GetSegment_internal(SQLite::Statement& query);
	std::optional<node::model::SocketNodeConnection> GetConnection_internal(SQLite::Statement& query);
	std::optional<node::model::NetModel> GetNet_internal(SQLite::Statement& query);

	std::string m_dbname;
	SQLite::Database& m_db;
	SubSceneId m_scene_id;
};

}