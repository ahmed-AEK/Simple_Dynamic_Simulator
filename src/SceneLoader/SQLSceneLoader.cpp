#include "SQLSceneLoader.hpp"
#include "toolgui/NodeMacros.h"
#include "SQLNodeLoader.hpp"
#include "SQLNetLoader.hpp"
#include <iostream>

std::optional<node::model::NodeSceneModel> node::loader::SQLSceneLoader::Load()
{
    std::optional<node::model::NodeSceneModel> scene{ std::in_place };
    SQLNodeLoader nodeLoader{ m_dbname, m_db };
    auto blocks = nodeLoader.GetBlocks();
    scene->ReserveBlocks(blocks.size());
    for (auto&& block : blocks)
    {
        scene->AddBlock(std::move(block));
    }
    return scene;
}

bool node::loader::SQLSceneLoader::Save(const node::model::NodeSceneModel& scene)
{
    try {

        m_db.exec("DROP TABLE IF EXISTS blocks");
        m_db.exec("DROP TABLE IF EXISTS sockets");
        m_db.exec("DROP TABLE IF EXISTS version");

        m_db.exec("CREATE TABLE version ( major INTEGER PRIMARY KEY, minor INTEGER )");
        m_db.exec("INSERT INTO version VALUES (0,1)");

        m_db.exec(R"(CREATE TABLE blocks (
                    id INTEGER PRIMARY KEY, 
                    x INTEGER NOT NULL, 
                    y INTEGER NOT NULL, 
                    w INTEGER NOT NULL, 
                    h INTEGER NOT NULL,
                    orientation INTEGER NOT NULL,
                    class_name TEXT NOT NULL,
                    styler_name TEXT NOT NULL);)");

        m_db.exec(R"(CREATE TABLE sockets (
                    id INTEGER NOT NULL,
                    parentid INTEGER NOT NULL, 
                    x INTEGER NOT NULL, 
                    y INTEGER NOT NULL, 
                    type INTEGER NOT NULL,
                    connected_node_id INTEGER,
                    PRIMARY KEY (id, parentid)
                    FOREIGN KEY (parentid) REFERENCES blocks(id) );)");

        m_db.exec(R"(CREATE TABLE blockProperties (
                    id INTEGER NOT NULL,
                    parentid INTEGER NOT NULL,
                    name TEXT NOT NULL,
                    type INTEGER NOT NULL,
                    value TEXT NOT NULL,
                    PRIMARY KEY (id, parentid),
                    FOREIGN KEY (parentid) REFERENCES blocks(id) );)");

        m_db.exec(R"(CREATE TABLE blockStylerProperties (
                    id INTEGER NOT NULL,
                    parentid INTEGER NOT NULL,
                    name TEXT NOT NULL,
                    value TEXT NOT NULL,
                    PRIMARY KEY (id, parentid),
                    FOREIGN KEY (parentid) REFERENCES blocks(id) );)");

        SQLNodeLoader nodeLoader{ m_dbname, m_db };
        for (const auto& node : scene.GetBlocks())
        {
            nodeLoader.AddBlock(node);
        }
        UNUSED_PARAM(scene);
        return true;
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<node::loader::NodeLoader> node::loader::SQLSceneLoader::GetBlockLoader()
{
    return m_nodeLoader;
}

std::shared_ptr<node::loader::NetLoader> node::loader::SQLSceneLoader::GetNetLoader()
{
    return m_netLoader;
}
