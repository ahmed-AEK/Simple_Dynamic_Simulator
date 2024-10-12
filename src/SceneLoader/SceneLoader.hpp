#pragma once

#include <memory>
#include "NodeModels/NodeScene.hpp"
#include "SceneLoader/NodeLoader.hpp"
#include "SceneLoader/NetLoader.hpp"

namespace node::loader
{


class SceneLoader
{
public:
	virtual std::optional<node::model::NodeSceneModel> Load() = 0;
	virtual bool Save(const node::model::NodeSceneModel& scene) = 0;
	virtual std::shared_ptr<NodeLoader> GetBlockLoader() = 0;
	virtual std::shared_ptr<NetLoader> GetNetLoader() = 0;

	virtual ~SceneLoader() = default;
	SceneLoader() = default;
	SceneLoader(SceneLoader&&) = default;
	SceneLoader(const SceneLoader&) = default;
	SceneLoader& operator=(SceneLoader&&) = default;
	SceneLoader& operator=(const SceneLoader&) = default;
};

}