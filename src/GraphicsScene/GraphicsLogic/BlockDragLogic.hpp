#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "GraphicsScene/GraphicsObject.hpp"

namespace node
{
class BlockObject;

namespace logic
{


class BlockDragLogic: public GraphicsLogic
{
public:
	BlockDragLogic(model::Point startPointMouseSpace, model::Point startObjectEdge, BlockObject& block, GraphicsScene* scene);
protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	void OnCancel() override;
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
private:
	model::Point m_startPointMouseSpace;
	model::Point m_startObjectEdge;
	node::HandlePtr<GraphicsObject> m_block;
	
};

}
}