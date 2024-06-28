#include "SQLSceneLoader.hpp"
#include "toolgui/NodeMacros.h"
#include "SQLNodeLoader.hpp"
#include <iostream>

std::optional<node::model::NodeSceneModel> node::loader::SQLSceneLoader::Load()
{
    std::optional<node::model::NodeSceneModel> scene{ std::in_place };
    SQLNodeLoader nodeLoader{ m_dbname, m_db };
    auto nodes = nodeLoader.GetNodes();
    (*scene).ReserveNodes(nodes.size());
    for (auto&& node : nodes)
    {
        (*scene).AddNode(std::move(node));
    }
    return scene;
}

bool node::loader::SQLSceneLoader::Save(const node::model::NodeSceneModel& scene)
{
    try {

        m_db.exec("DROP TABLE IF EXISTS nodes");
        m_db.exec("DROP TABLE IF EXISTS sockets");
        m_db.exec("DROP TABLE IF EXISTS version");

        m_db.exec("CREATE TABLE version ( major INTEGER PRIMARY KEY, minor INTEGER )");
        m_db.exec("INSERT INTO version VALUES (0,1)");

        m_db.exec(R"(CREATE TABLE nodes (
                    id INTEGER PRIMARY KEY, 
                    x INTEGER NOT NULL, 
                    y INTEGER NOT NULL, 
                    w INTEGER NOT NULL, 
                    h INTEGER NOT NULL);)");

        m_db.exec(R"(CREATE TABLE sockets (
                    id INTEGER NOT NULL,
                    parentid INTEGER NOT NULL, 
                    x INTEGER NOT NULL, 
                    y INTEGER NOT NULL, 
                    type INTEGER NOT NULL,
                    connected_node INTEGER,
                    PRIMARY KEY (id, parentid)
                    FOREIGN KEY (parentid) REFERENCES nodes(id) );)");

        SQLNodeLoader nodeLoader{ m_dbname, m_db };
        for (const auto& node : scene.GetNodes())
        {
            nodeLoader.AddNode(node);
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

std::unique_ptr<node::loader::NodeLoader> node::loader::SQLSceneLoader::GetNodeLoader()
{
    return std::make_unique<SQLNodeLoader>(m_dbname, m_db);
}
