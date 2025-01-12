#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "NodeModels/SceneModelManager.hpp"

namespace node
{

class NetNode;
class NetSegment;
class GraphicsObject;

namespace logic
{

struct NodeDragHandler
{
	struct AddedNode
	{
		AddedNode(NetModificationRequest::NodeIdType node_type,
			model::NetNodeId node_id,
			model::ConnectedSegmentSide connected_side)
			: node_type{ node_type }, node_id{ node_id }, connected_side{ connected_side } {}
		NetModificationRequest::NodeIdType node_type;
		model::NetNodeId node_id;
		model::ConnectedSegmentSide connected_side;
	};

	NodeDragHandler(const model::Point& start_point, NetNode& node);

	NetNode* GetBaseNode();
	const model::Point& GetNodeStartPosition() const { return m_node_start_position; }
	virtual void OnMove(const model::Point& new_point) = 0;
	virtual NetNode* GetRepresentingNode() = 0;
	virtual AddedNode CreateRequest(NetModificationRequest& request, const model::Point& last_point) = 0;
	virtual void OnCancel() = 0;
	model::Point m_start_point;
	HandlePtrS<NetNode, GraphicsObject> m_node;
	model::Point m_node_start_position;
	NodeDragHandler(NodeDragHandler&&) = default;
	NodeDragHandler(const NodeDragHandler&) = default;
	NodeDragHandler& operator=(const NodeDragHandler&) = default;
	NodeDragHandler& operator=(NodeDragHandler&&) = default;
	virtual ~NodeDragHandler() = default;
};

struct SegmentViewer
{
	SegmentViewer(NetNode& node1, NetNode& node2, NetSegment& segment);
	void Update(const model::Point& pos1, const model::Point& pos2);
	HandlePtrS<NetNode, GraphicsObject> m_first_node;
	HandlePtrS<NetNode, GraphicsObject> m_second_node;
	HandlePtrS<NetSegment, GraphicsObject> m_segment;
};

class VSegmentDragLogic : public GraphicsLogic
{
public:
	static std::unique_ptr<VSegmentDragLogic> Create(NetNode& node1, NetNode& node2, NetSegment& segment, 
		const model::Point& start_point, GraphicsScene* scene, GraphicsObjectsManager* manager);

	VSegmentDragLogic(SegmentViewer segment_viewer,
		std::unique_ptr<NetSegment> segment,
		std::unique_ptr<NodeDragHandler> first_node_handler,
		std::unique_ptr<NodeDragHandler> second_node_handler,
		std::vector<NetNode*> spare_nodes,
		std::vector<NetSegment*> spare_segments,
		GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	void OnCancel() override;
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;

private:

	void CleanUp();

	SegmentViewer m_segment_viewer;
	std::unique_ptr<NodeDragHandler> m_first_node_handler;
	std::unique_ptr<NodeDragHandler> m_second_node_handler;
	std::unique_ptr<NetSegment> m_base_segment;
	std::vector<NetNode*> m_spare_nodes;
	std::vector<NetSegment*> m_spare_segments;

};

}

}