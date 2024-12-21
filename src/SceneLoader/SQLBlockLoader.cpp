#include "SQLBlockLoader.hpp"
#include "toolgui/NodeMacros.h"
#include <algorithm>
#include <utility>

bool node::loader::SQLBlockLoader::AddBlock(const node::model::BlockModel& block)
{
	UNUSED_PARAM(block);
	
	{
		SQLite::Statement query{ m_db, "INSERT INTO blocks_" + std::to_string(m_scene_id.value) + " VALUES (?,?,?,?,?,?,?,?)"};
		query.bind(1, block.GetId().value);
		query.bind(2, block.GetBounds().x);
		query.bind(3, block.GetBounds().y);
		query.bind(4, block.GetBounds().w);
		query.bind(5, block.GetBounds().h);
		query.bind(6, static_cast<int>(block.GetOrienation()));
		query.bind(7, static_cast<int>(block.GetType()));
		query.bind(8, block.GetStyler());
		query.exec();
	}

	/*
	{
		auto&& properties = std::as_const(block).GetProperties();
		int32_t prop_id = 0;
		if (!std::all_of(properties.begin(), properties.end(),
			[&](const auto& property) { auto ret = AddProperty(block.GetId(), property, prop_id); prop_id++; return ret; }))
		{
			return false;
		}
	}
	*/

	{
		auto&& styler_properties = std::as_const(block).GetStylerProperties();
		int32_t prop_id = 0;
		if (!std::all_of(styler_properties.properties.begin(), styler_properties.properties.end(),
			[&](const auto& property) { auto ret = AddStylerProperty(block.GetId(), property.first, property.second, prop_id); prop_id++; return ret; }))
		{
			return false;
		}
	}

	auto&& sockets = block.GetSockets();
	if (!std::all_of(sockets.begin(), sockets.end(),
		[&](const auto& socket) { return AddSocket(socket, block.GetId()); }))
	{
		return false;
	}
	
	return true;
}

bool node::loader::SQLBlockLoader::AddBlockData(const node::model::BlockId& block_id, node::model::BlockType block_type, const node::model::NodeSceneModel& scene)
{
	if (block_type != model::BlockType::Functional)
	{
		return false;
	}
	auto* block_data = scene.GetFunctionalBlocksManager().GetDataForId(block_id);
	if (!block_data)
	{
		return false;
	}

	{
		SQLite::Statement query{ m_db, "INSERT INTO functionalBlockClass_" + std::to_string(m_scene_id.value) + " VALUES (?,?)" };
		query.bind(1, block_id.value);
		query.bindNoCopy(2, block_data->block_class);
		query.exec();
	}

	int32_t property_id = 0;
	for (const auto& property : block_data->properties)
	{
		SQLite::Statement query{ m_db, "INSERT INTO functionalBlockProperties_" + std::to_string(m_scene_id.value) + " VALUES (?,?,?,?,?)" };
		query.bind(1, property_id);
		query.bind(2, block_id.value);
		query.bind(3, property.name.c_str());
		query.bind(4, static_cast<int>(property.type));
		auto property_str = property.to_string();
		query.bind(5, property_str);
		query.exec();
		property_id++;
	}
	return true;
}

bool node::loader::SQLBlockLoader::DeleteBlockAndSockets(const node::model::BlockId& node_id)
{
	{
		SQLite::Statement query{ m_db, "DELETE FROM sockets_" + std::to_string(m_scene_id.value) + " WHERE parentid = ?" };
		query.bind(1, node_id.value);
		query.exec();
	}
	SQLite::Statement query{ m_db, "DELETE FROM blocks_" + std::to_string(m_scene_id.value) + " WHERE id = ?" };
	query.bind(1, node_id.value);
	query.exec();
	return true;
}

std::optional<node::model::BlockModel>
node::loader::SQLBlockLoader::GetBlock(const node::model::BlockId& block_id)
{
	using namespace node::model;
	SQLite::Statement query{ m_db, "SELECT * FROM blocks_" + std::to_string(m_scene_id.value) + " WHERE id = ?" };
	query.bind(1, block_id.value);
	if (query.executeStep())
	{
		if (auto block = GetBlock_internal(query))
		{
			return block;
		}
	}
	return std::nullopt;
}

std::optional<node::model::BlockData> node::loader::SQLBlockLoader::GetBlockData(model::BlockId block_id, model::BlockType block_type)
{
	if (block_type != model::BlockType::Functional)
	{
		return std::nullopt;
	}
	model::FunctionalBlockData data;
	
	{
		SQLite::Statement query{ m_db, "SELECT * FROM functionalBlockClass_" + std::to_string(m_scene_id.value) + " WHERE blockid = ?" };
		query.bind(1, block_id.value);
		if (query.executeStep())
		{
			data.block_class = query.getColumn(1).getString();
		}
	}

	{
		SQLite::Statement query{ m_db, "SELECT * FROM functionalBlockProperties_" + std::to_string(m_scene_id.value) + " WHERE parentid = ?" };
		query.bind(1, block_id.value);
		while (query.executeStep())
		{
			std::string name{ query.getColumn(2).getText() };
			int type_int{ query.getColumn(3) };
			assert(static_cast<size_t>(type_int) < std::variant_size_v<model::BlockProperty::property_t>);
			std::string property{ query.getColumn(4).getText() };
			auto type = static_cast<model::BlockPropertyType>(type_int);
			auto prop = model::BlockProperty::from_string(type, std::move(property));
			if (!prop)
			{
				return std::nullopt;
			}
			data.properties.push_back(model::BlockProperty{ std::move(name), type, std::move(*prop) });
		}
	}
	return node::model::BlockData{ data };

}


bool node::loader::SQLBlockLoader::UpdateBlockPosition(const node::model::BlockId& node_id,
	const node::model::Point& position)
{
	SQLite::Statement query{ m_db, "UPDATE blocks_" + std::to_string(m_scene_id.value) + " SET x = ?, y = ? WHERE id = ?" };
	query.bind(1, position.x);
	query.bind(2, position.y);
	query.bind(3, node_id.value);
	query.exec();
	return true;
}

bool node::loader::SQLBlockLoader::UpdateBlockBounds(const node::model::BlockId& node_id,
	const node::model::Rect& bounds)
{
	SQLite::Statement query{ m_db, "UPDATE blocks_" + std::to_string(m_scene_id.value) + " SET x = ?, y = ?, w = ?, h = ? WHERE id = ?" };
	query.bind(1, bounds.x);
	query.bind(2, bounds.y);
	query.bind(3, bounds.w);
	query.bind(4, bounds.h);
	query.bind(5, node_id.value);
	query.exec();
	return true;
}

bool node::loader::SQLBlockLoader::AddStylerProperty(const node::model::BlockId& block_id, const std::string& name, const std::string& value, int32_t property_id)
{
	SQLite::Statement query{ m_db, "INSERT INTO blockStylerProperties_" + std::to_string(m_scene_id.value) + " VALUES (?,?,?,?)" };
	query.bind(1, property_id);
	query.bind(2, block_id.value);
	query.bind(3, name.data());
	query.bind(4, value.data());
	query.exec();
	return true;
}

bool node::loader::SQLBlockLoader::AddProperty(const node::model::BlockId& block_id, const model::BlockProperty& property, int32_t property_id)
{
	SQLite::Statement query{ m_db, "INSERT INTO functionalBlockProperties_" + std::to_string(m_scene_id.value) + " VALUES (?,?,?,?,?)" };
	query.bind(1, property_id);
	query.bind(2, block_id.value);
	query.bind(3, property.name.c_str());
	query.bind(4, static_cast<int>(property.type));
	auto property_str = property.to_string();
	query.bind(5, property_str);
	query.exec();
	return true;
}

bool node::loader::SQLBlockLoader::AddSocket(const node::model::BlockSocketModel& socket,
	const node::model::BlockId& block_id)
{
	SQLite::Statement querySocket{ m_db, "INSERT INTO sockets_" + std::to_string(m_scene_id.value) + " VALUES (?,?,?,?,?,?)" };
	querySocket.bind(1, socket.GetId().value);
	querySocket.bind(2, block_id.value);
	querySocket.bind(3, socket.GetPosition().x);
	querySocket.bind(4, socket.GetPosition().y);
	querySocket.bind(5, static_cast<int>(socket.GetType()));
	if (auto val = socket.GetConnectedNetNode(); val)
	{
		querySocket.bind(6, val->value);
	}
	else
	{
		querySocket.bind(6);
	}
	querySocket.exec();
	return true;
}

bool node::loader::SQLBlockLoader::DeleteSocket(const node::model::SocketUniqueId& socket_id)
{
	SQLite::Statement query{ m_db, "DELETE FROM sockets_" + std::to_string(m_scene_id.value) + " WHERE id = ? AND parentid = ?" };
	query.bind(1, socket_id.socket_id.value);
	query.bind(2, socket_id.block_id.value);
	query.exec();
	return true;
}

bool node::loader::SQLBlockLoader::UpdateSocketPosition(const node::model::SocketUniqueId& socket_id,
	const node::model::Point& position)
{
	SQLite::Statement query{ m_db, "UPDATE sockets_" + std::to_string(m_scene_id.value) + " SET x = ?, y = ? WHERE id = ? AND parentid = ?" };
	query.bind(1, position.x);
	query.bind(2, position.y);
	query.bind(3, socket_id.socket_id.value);
	query.bind(4, socket_id.block_id.value);
	query.exec();
	return true;
}

node::model::BlockId node::loader::SQLBlockLoader::GetNextBlockId()
{
	SQLite::Statement query{ m_db, "SELECT MAX(id) FROM blocks_" + std::to_string(m_scene_id.value) };
	auto result = query.executeStep();
	UNUSED_PARAM(result);
	assert(result);
	return model::BlockId{ static_cast<node::model::id_int>(query.getColumn(0)) + 1 };
}

bool node::loader::SQLBlockLoader::GetBlocks(std::vector<node::model::BlockModel>& blocks)
{
	using namespace node::model;

	SQLite::Statement query{ m_db, "SELECT * FROM blocks_" + std::to_string(m_scene_id.value) };
	while (query.executeStep())
	{
		if (auto block = GetBlock_internal(query))
		{
			blocks.push_back(std::move(*block));
		}
	}
	return true;
}

void
node::loader::SQLBlockLoader::LoadSocketsForBlock(node::model::BlockModel& block)
{
	using namespace node::model;
	SQLite::Statement querySocket{ m_db, "SELECT * FROM sockets_" + std::to_string(m_scene_id.value) + " WHERE parentid = ?" };
	querySocket.bind(1, block.GetId().value);
	while (querySocket.executeStep())
	{
		id_int socket_id = querySocket.getColumn(0);

		Point socketOrigin{ querySocket.getColumn(2), querySocket.getColumn(3) };
		BlockSocketModel::SocketType type =
			static_cast<node::model::BlockSocketModel::SocketType>(
				static_cast<int>(querySocket.getColumn(4)));
		node::model::BlockSocketModel socket{ type, model::SocketId{socket_id}, socketOrigin };
		auto connected_node_column = querySocket.getColumn(5);
		if (!connected_node_column.isNull())
		{
			socket.SetConnectedNetNode(NetNodeId{ static_cast<id_int>(connected_node_column) });
		}
		block.AddSocket(std::move(socket));
	}
}

bool node::loader::SQLBlockLoader::LoadStylerProperties(node::model::BlockModel& block)
{
	using namespace node::model;

	SQLite::Statement propertyquery{ m_db, "SELECT * FROM blockStylerProperties_" + std::to_string(m_scene_id.value) + " WHERE parentid = ?" };
	propertyquery.bind(1, block.GetId().value);
	BlockStyleProperties properties;
	while (propertyquery.executeStep())
	{
		std::string name{ propertyquery.getColumn(2).getText() };
		std::string value{ propertyquery.getColumn(3).getText()};
		properties.properties.emplace(std::move(name), std::move(value));
	}
	block.SetStylerProperties(properties);
	return true;
}

std::optional<node::model::BlockModel> node::loader::SQLBlockLoader::GetBlock_internal(SQLite::Statement& query)
{	
	using namespace node::model;

	BlockId block_id{ query.getColumn(0) };
	Rect bounds{ query.getColumn(1), query.getColumn(2),
		query.getColumn(3), query.getColumn(4) };
	int orientation_value = static_cast<int>(query.getColumn(5));
	assert(orientation_value < 4);
	BlockOrientation orientation = static_cast<BlockOrientation>(orientation_value);
	model::BlockType block_type = static_cast<model::BlockType>(query.getColumn(6).getInt());
	BlockModel block{ block_id, block_type, bounds, orientation };
	std::string styler_name{ query.getColumn(7).getText() };
	block.SetStyler(std::move(styler_name));
	LoadStylerProperties(block);
	LoadSocketsForBlock(block);	
	return block;
}
