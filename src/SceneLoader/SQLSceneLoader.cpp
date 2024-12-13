#include "SQLSceneLoader.hpp"
#include "toolgui/NodeMacros.h"
#include "SQLBlockLoader.hpp"
#include "SQLNetLoader.hpp"
#include <iostream>

node::loader::SQLSceneLoader::SQLSceneLoader(std::string dbname)
    : m_dbname{ std::move(dbname)} {
}

bool node::loader::SQLSceneLoader::Reset()
{
    try
    {
        if (!m_db)
        {
            m_db = SQLite::Database{ m_dbname , SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE };
        }

        m_db->exec("PRAGMA synchronous = OFF;");

        std::vector<std::string> tables;
        {
            SQLite::Statement query{ *m_db, "SELECT name FROM sqlite_schema WHERE type='table';" };
            while (query.executeStep())
            {
                auto col_name = query.getColumn(0).getString();
                tables.push_back(std::move(col_name));
            }
        }
        {
            for (const auto& col_name : tables)
            {
                m_db->exec("DROP TABLE " + col_name);
            }
        }

        m_db->exec("CREATE TABLE version ( major INTEGER PRIMARY KEY, minor INTEGER )");
        m_db->exec("INSERT INTO version VALUES (0,1)");
        m_db->exec(R"(CREATE TABLE SubSystems (
                     id INTEGER PRIMARY KEY,
                     parent_id INTEGER NOT NULL);)");
    }
    catch (SQLite::Exception& e)
    {
        std::cout << "EXCEPTION: SQLITE CODE: " << e.getErrorCode() <<
            " , DESCRIPTION: " << e.what() << '\n';
        return false;
    }
    return true;

}

std::optional<node::model::NodeSceneModel> node::loader::SQLSceneLoader::Load(SubSceneId id)
{
    try
    {
        if (!m_db)
        {
            m_db = SQLite::Database{ m_dbname , SQLite::OPEN_READWRITE };
        }
        std::optional<node::model::NodeSceneModel> scene{ std::in_place };
        SQLBlockLoader nodeLoader{ m_dbname, *m_db , id};
        auto blocks = nodeLoader.GetBlocks();
        scene->ReserveBlocks(blocks.size());
        for (auto&& block : blocks)
        {
            scene->AddBlock(std::move(block));
        }

        SQLNetLoader netLoader{ m_dbname, *m_db, id };
        for (auto&& net_node : netLoader.GetNetNodes())
        {
            scene->AddNetNode(std::move(net_node));
        }
        for (auto&& net_segment : netLoader.GetNetSegments())
        {
            scene->AddNetSegment(std::move(net_segment));
        }
        for (auto&& socket_connection : netLoader.GetSocketNodeConnections())
        {
            scene->AddSocketNodeConnection(std::move(socket_connection));
        }
        return scene;
    }
    catch (SQLite::Exception& e)
    {
        std::cout << "EXCEPTION: SQLITE CODE: " << e.getErrorCode() << 
            " , DESCRIPTION: " << e.what() << '\n';
        return std::nullopt;
    }
}

std::optional<std::vector<node::SubSceneId>> node::loader::SQLSceneLoader::GetChildSubScenes(SubSceneId id)
{
    std::vector<SubSceneId> ids;
    try
    {
        {
            SQLite::Statement query{ *m_db, "SELECT id FROM SubSystems WHERE parent_id = ?" };
            query.bind(1, id.value);
            while (query.executeStep())
            {
                ids.push_back(SubSceneId{ query.getColumn(0) });
            }
        }
    }
    catch (SQLite::Exception& e)
    {
        std::cout << "EXCEPTION: SQLITE CODE: " << e.getErrorCode() <<
            " , DESCRIPTION: " << e.what() << '\n';
        return std::nullopt;
    }
    return {std::move(ids)};
}

bool node::loader::SQLSceneLoader::Save(const node::model::NodeSceneModel& scene, SubSceneId id, SubSceneId parent_id)
{
    try {
        if (!m_db)
        {
            m_db = SQLite::Database{ m_dbname , SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE };
            Reset();
        }

        m_db->exec("PRAGMA synchronous = OFF;");
        
        {
            SQLite::Statement query{ *m_db, "INSERT INTO SubSystems VALUES (?, ?)" };
            query.bind(1, id.value);
            query.bind(2, parent_id.value);
            query.exec();
        }

        m_db->exec(R"(CREATE TABLE blocks_)" 
                    + std::to_string(id.value) + R"((
                    id INTEGER PRIMARY KEY, 
                    x INTEGER NOT NULL, 
                    y INTEGER NOT NULL, 
                    w INTEGER NOT NULL, 
                    h INTEGER NOT NULL,
                    orientation INTEGER NOT NULL,
                    class_name TEXT NOT NULL,
                    styler_name TEXT NOT NULL);)");

        m_db->exec(R"(CREATE TABLE sockets_)"
                    + std::to_string(id.value) + R"((
                    id INTEGER NOT NULL,
                    parentid INTEGER NOT NULL, 
                    x INTEGER NOT NULL, 
                    y INTEGER NOT NULL, 
                    type INTEGER NOT NULL,
                    connected_node_id INTEGER,
                    PRIMARY KEY (id, parentid)
                    FOREIGN KEY (parentid) REFERENCES blocks(id) );)");

        m_db->exec(R"(CREATE TABLE blockProperties_)"
                    + std::to_string(id.value) + R"((
                    id INTEGER NOT NULL,
                    parentid INTEGER NOT NULL,
                    name TEXT NOT NULL,
                    type INTEGER NOT NULL,
                    value TEXT NOT NULL,
                    PRIMARY KEY (id, parentid),
                    FOREIGN KEY (parentid) REFERENCES blocks(id) );)");

        m_db->exec(R"(CREATE TABLE blockStylerProperties_)"
                    + std::to_string(id.value) + R"((
                    id INTEGER NOT NULL,
                    parentid INTEGER NOT NULL,
                    name TEXT NOT NULL,
                    value TEXT NOT NULL,
                    PRIMARY KEY (id, parentid),
                    FOREIGN KEY (parentid) REFERENCES blocks(id) );)");

        m_db->exec(R"(CREATE TABLE NetNodes_)" 
                    + std::to_string(id.value) + R"((
                    id INTEGER PRIMARY KEY,
                    x INTEGER NOT NULL,
                    y INTEGER NOT NULL,
                    north_segment INTEGER,
                    south_segment INTEGER,
                    west_segment INTEGER, 
                    east_segment INTEGER );)");

        m_db->exec(R"(CREATE TABLE NetSegments_)" 
                    + std::to_string(id.value) + R"((
                    id INTEGER PRIMARY KEY,
                    first_segment INTEGER NOT NULL,
                    second_segment INTEGER NOT NULL,
                    orientation INTEGER NOT NULL );)");

        m_db->exec(R"(CREATE TABLE SocketNodeConnections_)" 
                    + std::to_string(id.value) + R"((
                    socket_id INTEGER NOT NULL,
                    block_id INTEGER NOT NULL,
                    node_id INTEGER NOT NULL,
                    PRIMARY KEY (socket_id, block_id, node_id) );)");

        SQLBlockLoader nodeLoader{ m_dbname, *m_db, id };
        for (const auto& block : scene.GetBlocks())
        {
            nodeLoader.AddBlock(block);
        }

        SQLNetLoader netLoader{ m_dbname, *m_db, id };
        for (const auto& net_node : scene.GetNetNodes())
        {
            netLoader.AddNetNode(net_node);
        }
        for (const auto& net_segment : scene.GetNetSegments())
        {
            netLoader.AddNetSegment(net_segment);
        }
        for (const auto& connection : scene.GetSocketConnections())
        {
            netLoader.AddSocketNodeConnection(connection);
        }
        return true;
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<node::loader::BlockLoader> node::loader::SQLSceneLoader::GetBlockLoader(SubSceneId id)
{
    UNUSED_PARAM(id);
    assert(false);
    return nullptr;
}

std::shared_ptr<node::loader::NetLoader> node::loader::SQLSceneLoader::GetNetLoader(SubSceneId id)
{
    UNUSED_PARAM(id);
    assert(false);
    return nullptr;
}
