#pragma once

#include <memory>
#include "NodeModels/NodeScene.hpp"
#include "SceneLoader/BlockLoader.hpp"
#include "SceneLoader/NetLoader.hpp"



namespace node::loader
{

class SceneLoader
{
public:
	virtual bool Reset() = 0;
	virtual std::optional<node::model::NodeSceneModel> Load(SubSceneId id) = 0;
	virtual std::optional<std::vector<SubSceneId>> GetChildSubScenes(SubSceneId id) = 0;
	virtual bool Save(const node::model::NodeSceneModel& scene, SubSceneId id, SubSceneId parent_id) = 0;
	virtual std::shared_ptr<BlockLoader> GetBlockLoader(SubSceneId id) = 0;
	virtual std::shared_ptr<NetLoader> GetNetLoader(SubSceneId id) = 0;
	virtual std::string GetSceneName() const = 0;

	virtual ~SceneLoader() = default;
	SceneLoader() = default;
	SceneLoader(SceneLoader&&) = default;
	SceneLoader(const SceneLoader&) = default;
	SceneLoader& operator=(SceneLoader&&) = default;
	SceneLoader& operator=(const SceneLoader&) = default;
};

std::unique_ptr<SceneLoader> MakeSqlLoader(std::string_view db_path);

}