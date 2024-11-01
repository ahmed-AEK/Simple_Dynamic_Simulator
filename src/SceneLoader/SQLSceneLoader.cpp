#include "SQLSceneLoader.hpp"
#include "toolgui/NodeMacros.h"
#include "SQLBlockLoader.hpp"
#include "SQLNetLoader.hpp"
#include <iostream>

node::loader::SQLSceneLoader::SQLSceneLoader(std::string dbname)
    : m_dbname{ std::move(dbname)} {
}

std::optional<node::model::NodeSceneModel> node::loader::SQLSceneLoader::Load()
{
    try
    {
        if (!m_db)
        {
            m_db = SQLite::Database{ m_dbname , SQLite::OPEN_READWRITE };
        }
        std::optional<node::model::NodeSceneModel> scene{ std::in_place };
        SQLBlockLoader nodeLoader{ m_dbname, *m_db };
        auto blocks = nodeLoader.GetBlocks();
        scene->ReserveBlocks(blocks.size());
        for (auto&& block : blocks)
        {
            scene->AddBlock(std::move(block));
        }

        SQLNetLoader netLoader{ m_dbname, *m_db };
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

bool node::loader::SQLSceneLoader::Save(const node::model::NodeSceneModel& scene)
{
    try {
        if (!m_db)
        {
            m_db = SQLite::Database{ m_dbname , SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE };
        }

        m_db->exec("PRAGMA synchronous = OFF;");

        m_db->exec("DROP TABLE IF EXISTS blocks");
        m_db->exec("DROP TABLE IF EXISTS sockets");
        m_db->exec("DROP TABLE IF EXISTS blockProperties");
        m_db->exec("DROP TABLE IF EXISTS blockStylerProperties");
        m_db->exec("DROP TABLE IF EXISTS NetNodes");
        m_db->exec("DROP TABLE IF EXISTS NetSegments");
        m_db->exec("DROP TABLE IF EXISTS SocketNodeConnections");
        m_db->exec("DROP TABLE IF EXISTS version");

        m_db->exec("CREATE TABLE version ( major INTEGER PRIMARY KEY, minor INTEGER )");
        m_db->exec("INSERT INTO version VALUES (0,1)");

        m_db->exec(R"(CREATE TABLE blocks (
                    id INTEGER PRIMARY KEY, 
                    x INTEGER NOT NULL, 
                    y INTEGER NOT NULL, 
                    w INTEGER NOT NULL, 
                    h INTEGER NOT NULL,
                    orientation INTEGER NOT NULL,
                    class_name TEXT NOT NULL,
                    styler_name TEXT NOT NULL);)");

        m_db->exec(R"(CREATE TABLE sockets (
                    id INTEGER NOT NULL,
                    parentid INTEGER NOT NULL, 
                    x INTEGER NOT NULL, 
                    y INTEGER NOT NULL, 
                    type INTEGER NOT NULL,
                    connected_node_id INTEGER,
                    PRIMARY KEY (id, parentid)
                    FOREIGN KEY (parentid) REFERENCES blocks(id) );)");

        m_db->exec(R"(CREATE TABLE blockProperties (
                    id INTEGER NOT NULL,
                    parentid INTEGER NOT NULL,
                    name TEXT NOT NULL,
                    type INTEGER NOT NULL,
                    value TEXT NOT NULL,
                    PRIMARY KEY (id, parentid),
                    FOREIGN KEY (parentid) REFERENCES blocks(id) );)");

        m_db->exec(R"(CREATE TABLE blockStylerProperties (
                    id INTEGER NOT NULL,
                    parentid INTEGER NOT NULL,
                    name TEXT NOT NULL,
                    value TEXT NOT NULL,
                    PRIMARY KEY (id, parentid),
                    FOREIGN KEY (parentid) REFERENCES blocks(id) );)");

        m_db->exec(R"(CREATE TABLE NetNodes (
                    id INTEGER PRIMARY KEY,
                    x INTEGER NOT NULL,
                    y INTEGER NOT NULL,
                    north_segment INTEGER,
                    south_segment INTEGER,
                    west_segment INTEGER, 
                    east_segment INTEGER );)");

        m_db->exec(R"(CREATE TABLE NetSegments (
                    id INTEGER PRIMARY KEY,
                    first_segment INTEGER NOT NULL,
                    second_segment INTEGER NOT NULL,
                    orientation INTEGER NOT NULL );)");

        m_db->exec(R"(CREATE TABLE SocketNodeConnections (
                    socket_id INTEGER NOT NULL,
                    block_id INTEGER NOT NULL,
                    node_id INTEGER NOT NULL,
                    PRIMARY KEY (socket_id, block_id, node_id) );)");

        SQLBlockLoader nodeLoader{ m_dbname, *m_db };
        for (const auto& block : scene.GetBlocks())
        {
            nodeLoader.AddBlock(block);
        }

        SQLNetLoader netLoader{ m_dbname, *m_db };
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

std::shared_ptr<node::loader::BlockLoader> node::loader::SQLSceneLoader::GetBlockLoader()
{
    return nullptr;
}

std::shared_ptr<node::loader::NetLoader> node::loader::SQLSceneLoader::GetNetLoader()
{
    return nullptr;
}
