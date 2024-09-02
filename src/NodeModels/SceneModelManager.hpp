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
	model::NetNodeId moved_node;
	model::NetNodeId second_moved_node;
	model::Point new_position;
};

struct NetModificationReport
{
	struct UpdateNodeReport
	{
		model::NetNodeId node_id;
		model::Point new_position;
	};

	std::vector<model::SocketUniqueId> removed_connections;
	std::vector<model::NetSegmentId> removed_segments;
	std::vector<model::NetNodeId> removed_nodes;
	std::vector<std::reference_wrapper<model::NetNodeModel>> added_nodes;
	std::vector<UpdateNodeReport> update_nodes;
	std::vector<std::reference_wrapper<model::NetSegmentModel>> update_segments;
	std::vector<std::reference_wrapper<model::NetSegmentModel>> added_segments;
	std::vector<std::reference_wrapper<model::SocketNodeConnection>> added_connections;
};

enum class SceneModificationType
{
	BlockAdded,
	BlockRemoved,
	BlockMoved,
	NetAdded,
	LeafNodeMoved,
	NetUpdated,
};

struct SceneModification
{
	using type_t = SceneModificationType;
	using data_t = std::variant<model::BlockModelRef, model::BlockId, model::NetModelRef, LeafNodeMovedReport, std::reference_wrapper<NetModificationReport>>;
	SceneModificationType type;
	data_t data;
};


struct NetModificationRequest
{
	enum class NodeIdType
	{
		existing_id,
		new_id,
	};

	struct AddNodeRequest
	{
		model::Point position;
	};

	struct AddSegmentRequest
	{
		NodeIdType node1_type;
		NodeIdType node2_type;
		model::ConnectedSegmentSide node1_side;
		model::ConnectedSegmentSide node2_side;
		model::NetSegmentOrientation orientation;
		model::NetNodeId node1;
		model::NetNodeId node2;
	};

	struct UpdateNodeRequest
	{
		model::NetNodeId node_id;
		model::Point new_position;
	};

	struct UpdateSegmentRequest
	{
		NodeIdType node1_type;
		NodeIdType node2_type;
		model::ConnectedSegmentSide node1_side;
		model::ConnectedSegmentSide node2_side;
		model::NetSegmentId segment_id;
		model::NetNodeId node1;
		model::NetNodeId node2;
	};

	struct SocketConnectionRequest
	{
		model::SocketUniqueId socket;
		NodeIdType node_type;
		model::NetNodeId node;
	};

	std::vector<model::SocketUniqueId> removed_connections;
	std::vector<model::NetSegmentId> removed_segments;
	std::vector<model::NetNodeId> removed_nodes;
	std::vector<AddNodeRequest> added_nodes;
	std::vector<UpdateNodeRequest> update_nodes;
	std::vector<UpdateSegmentRequest> update_segments;
	std::vector<AddSegmentRequest> added_segments;
	std::vector<SocketConnectionRequest> added_connections;
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

	void ModifyBlockProperties(model::BlockId id, std::vector<model::BlockProperty> new_properties);

	void AddNewNet(model::NetModel&& net);
	void UpdateNet(NetModificationRequest& update_request);
	model::NodeSceneModel& GetModel() { return *m_scene; }
	const model::NodeSceneModel& GetModel() const { return *m_scene; }
private:
	std::shared_ptr<model::NodeSceneModel> m_scene;
};

}