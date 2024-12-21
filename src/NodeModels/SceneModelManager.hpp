#pragma once

#include "NodeModels/NodeScene.hpp"
#include "NodeModels/Observer.hpp"
#include "NodeModels/FunctionalBlocksManager.hpp"
#include <variant>
#include <stack>

namespace node
{

class SceneModelManager;

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
	BlockAddedWithConnections,
	BlockRemoved,
	BlockMoved,
	BlockResized,
	NetUpdated,
	BlockPropertiesModified,
	BlockPropertiesAndSocketsModified,
};

struct BlockAddWithConnectionsReport
{
	model::BlockModelConstRef block;
	std::span<const model::SocketNodeConnection> connections;
};

struct SceneModification
{
	using type_t = SceneModificationType;
	using data_t = std::variant<model::BlockModelConstRef, model::BlockId, model::NetModelRef, LeafNodeMovedReport, std::reference_wrapper<NetModificationReport>, BlockAddWithConnectionsReport>;
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

class ModelAction
{
public:
	bool Undo(SceneModelManager& manager) { return DoUndo(manager); }
	bool Redo(SceneModelManager& manager) { return DoRedo(manager);  }

	ModelAction() = default;
	virtual ~ModelAction() = default;
	ModelAction(const ModelAction&) = default;
	ModelAction(ModelAction&&) = default;
	ModelAction& operator=(const ModelAction&) = default;
	ModelAction& operator=(ModelAction&&) = default;
protected:
	virtual bool DoUndo(SceneModelManager& manager) = 0;
	virtual bool DoRedo(SceneModelManager& manager) = 0;

};

class SceneModelManager : public MultiPublisher<SceneModification>
{
public:
	explicit SceneModelManager(std::shared_ptr<model::NodeSceneModel> scene);
	~SceneModelManager() override;

	std::span<model::BlockModel> GetBlocks();
	void AddNewFunctionalBlock(model::BlockModel&& block, model::FunctionalBlockData&& data);
	void RemoveBlockById(const model::BlockId& id);
	void MoveBlockById(const model::BlockId& id, const model::Point& new_origin);
	void ResizeBlockById(const model::BlockId& id, const model::Rect& new_rect, model::BlockOrientation new_orientation, std::vector<model::BlockSocketModel> socket_positions);

	void ModifyBlockProperties(model::BlockId id, std::vector<model::BlockProperty> new_properties);
	void ModifyBlockPropertiesAndSockets(model::BlockId id, std::vector<model::BlockProperty> new_properties, std::vector<model::BlockSocketModel> new_sockets);

	void UpdateNet(NetModificationRequest& update_request);
	model::NodeSceneModel& GetModel() { return *m_scene; }
	const model::NodeSceneModel& GetModel() const { return *m_scene; }

	bool CanUndo() const { return m_undo_stack.size(); }
	bool CanRedo() const { return m_redo_stack.size(); }
	void Undo();
	void Redo();
	void PushAction(std::unique_ptr<ModelAction> action);

	SubSceneId GetSubSceneId() const { return m_id; };
	void SetSubSceneId(SubSceneId id) { m_id = id; }

	SubSceneId GetParentSceneId() const { return m_id; };
	void SetParentSceneId(SubSceneId id) { m_id = id; }

	const auto& GetSubsystemIds() const { return m_subsystem_ids; }
private:
	SubSceneId m_id;
	SubSceneId m_parent_id;
	std::shared_ptr<model::NodeSceneModel> m_scene;
	std::stack<std::unique_ptr<ModelAction>> m_undo_stack;
	std::stack<std::unique_ptr<ModelAction>> m_redo_stack;
	std::unordered_map<model::BlockId, SubSceneId> m_subsystem_ids;
};

}