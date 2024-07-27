#pragma once

#include "NodeModels/NodeScene.hpp"
#include "NodeModels/BlockModel.hpp"
#include "NodeModels/Observer.hpp"
#include <variant>

namespace node
{

enum class SceneModificationType
{
	BlockAdded,
	BlockRemoved,
};

struct SceneModification
{
	using type_t = SceneModificationType;
	using data_t = std::variant<model::BlockModelPtr>;
	SceneModificationType type;
	data_t data;
};

class SceneModelManager : public MultiPublisher<SceneModification>
{
public:
	SceneModelManager(std::shared_ptr<model::NodeSceneModel> scene);
	~SceneModelManager();

	std::span<model::BlockModelPtr> GetBlocks();
	void AddBlock(model::BlockModelPtr block);
	void RemoveBlockById(model::id_int id);
private:
	std::shared_ptr<model::NodeSceneModel> m_scene;
};

}