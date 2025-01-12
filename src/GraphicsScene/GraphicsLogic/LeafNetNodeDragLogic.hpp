#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"

namespace node
{
class NetNode;
class BlockSocketObject;
class GraphicsObject;

namespace logic
{

class LeafNetNodeDragLogic : public GraphicsLogic
{
public:
	static std::unique_ptr<LeafNetNodeDragLogic> TryCreate(NetNode& node,
		GraphicsScene& scene, GraphicsObjectsManager& manager);
	LeafNetNodeDragLogic(NetNode& dragged_node, NetNode& connected_node,
		GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
	void OnCancel() override;
private:
	BlockSocketObject* GetSocketAt(const model::Point& point) const;
	void CleanUp();

	model::Point m_first_node_start_point;
	model::Point m_second_node_start_point;
	HandlePtrS<NetNode, GraphicsObject> m_dragged_node;
	HandlePtrS<NetNode, GraphicsObject> m_connected_node;
};
}
}