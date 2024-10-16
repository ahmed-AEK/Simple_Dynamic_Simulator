#include "SceneLoader.hpp"
#include "SQLSceneLoader.hpp"

std::unique_ptr<node::loader::SceneLoader> node::loader::MakeSqlLoader(std::string_view db_path)
{
    return std::make_unique<SQLSceneLoader>(std::string{ db_path });
}
