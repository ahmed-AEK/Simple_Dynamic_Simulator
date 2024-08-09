#pragma once

#include "SceneLoader/NetLoader.hpp"
#include "SQLiteCpp/Database.h"

namespace node::loader
{


class SQLNetLoader: public NetLoader
{
public:
	SQLNetLoader(std::string dbname, SQLite::Database& db)
		: m_dbname{ dbname }, m_db{ db } {}

	// NetNode Functions
	bool AddNetNode(const node::model::NetNodeModel& node) override;
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
	virtual bool AddNetSegment(
		const node::model::NetSegmentModel& segment);
	virtual bool DeleteNetSegment(const node::model::id_int id);
	virtual bool UpdateNetSegment(
		const node::model::NetSegmentModel& segment);
	virtual std::optional<node::model::NetSegmentModel> GetSegment(
		const node::model::id_int id);
	virtual node::model::id_int GetNextSegmentId();

	// Net Functions
	virtual bool AddNet(const node::model::NetModel& net);
	virtual bool DeleteNet(const node::model::id_int id);
	virtual std::optional<node::model::NetModel> GetNet(
		const node::model::id_int id);
	virtual bool SetNetName(const node::model::id_int id,
		std::string_view name);
	virtual bool AddNodeToNet(const node::model::id_int net_id,
		const node::model::id_int node_id);
	virtual bool RemoveNodeFromNet(const node::model::id_int net_id,
		const node::model::id_int node_id);
	virtual bool AddSegmentToNet(const node::model::id_int net_id,
		const node::model::id_int segment_id);
	virtual bool RemoveSegmentFromNet(const node::model::id_int net_id,
		const node::model::id_int segment_id);
	virtual node::model::id_int GetNextNetId();

private:
	std::string m_dbname;
	SQLite::Database& m_db;
};

}