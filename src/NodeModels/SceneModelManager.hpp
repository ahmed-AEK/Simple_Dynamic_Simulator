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
	BlockMoved,
	NetAdded,
};

struct SceneModification
{
	using type_t = SceneModificationType;
	using data_t = std::variant<model::BlockModelRef, model::BlockId, model::NetModelRef>;
	SceneModificationType type;
	data_t data;
};

class SceneModelManager : public MultiPublisher<SceneModification>
{
public:
	explicit SceneModelManager(std::shared_ptr<model::NodeSceneModel> scene);
	~SceneModelManager() override;

	std::span<model::BlockModel> GetBlocks();
	void AddNewBlock(model::BlockModel&& block);
	void RemoveBlockById(const model::BlockId& id);
	void MoveBlockById(const model::BlockId& id, const model::Point& new_origin);

	void AddNewNet(model::NetModel&& net);

private:
	std::shared_ptr<model::NodeSceneModel> m_scene;
};

}