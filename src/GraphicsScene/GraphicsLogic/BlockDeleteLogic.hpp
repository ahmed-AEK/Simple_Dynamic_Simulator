#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"

namespace node
{

class GraphicsObject;
class BlockObject;

namespace logic
{

class BlockDeleteLogic : public GraphicsLogic
{
public:
	BlockDeleteLogic(BlockObject& block, GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	virtual void OnMouseMove(const model::Point& current_mouse_point) override;
	virtual MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
private:
	HandlePtrS<BlockObject, GraphicsObject> m_block;
};

}
}