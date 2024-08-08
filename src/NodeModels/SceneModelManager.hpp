#pragma once

#include "NodeModels/NodeScene.hpp"
#include "NodeModels/BlockModel.hpp"
#include "NodeModels/Observer.hpp"
#include <variant>

namespace node
{


struct LeafNodeMovedReport
{
	std::optional<model::SocketUniqueId> new_socket;
	model::NetNodeUniqueId moved_node;
	model::NetNodeUniqueId second_moved_node;
	model::Point new_position;
};

enum class SceneModificationType
{
	BlockAdded,
	BlockRemoved,
	BlockMoved,
	NetAdded,
	LeafNodeMoved,
};

struct SceneModification
{
	using type_t = SceneModificationType;
	using data_t = std::variant<model::BlockModelRef, model::BlockId, model::NetModelRef, LeafNodeMovedReport>;
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
	void MoveLeafNetNode(model::NetNodeUniqueId main_node_id, model::NetNodeUniqueId second_node_id, model::Point new_position,
		std::optional<model::SocketUniqueId> connected_socket);
private:
	std::shared_ptr<model::NodeSceneModel> m_scene;
};

}