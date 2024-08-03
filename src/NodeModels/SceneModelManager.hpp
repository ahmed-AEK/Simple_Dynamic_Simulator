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
	NetAdded,
};

struct SceneModification
{
	using type_t = SceneModificationType;
	using data_t = std::variant<model::BlockModelPtr, model::NetModelPtr>;
	SceneModificationType type;
	data_t data;
};

class SceneModelManager : public MultiPublisher<SceneModification>
{
public:
	explicit SceneModelManager(std::shared_ptr<model::NodeSceneModel> scene);
	~SceneModelManager() override;

	std::span<model::BlockModelPtr> GetBlocks();
	void AddNewBlock(model::BlockModelPtr block);
	void AddNewNet(model::NetModelPtr net);
	void RemoveBlockById(const model::BlockId& id);
private:
	std::shared_ptr<model::NodeSceneModel> m_scene;
};

}