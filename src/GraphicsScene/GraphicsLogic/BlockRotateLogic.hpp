#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "GraphicsScene/GraphicsObject.hpp"

namespace node
{

class BlockObject;

namespace logic
{
class BlockRotateLogic : public GraphicsLogic
{
public:
	BlockRotateLogic(const model::Rect& rotate_btn_rect, BlockObject& block,GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;

private:
	model::Rect m_rotate_btn_rect;
	HandlePtr<GraphicsObject> m_block;
};
}
}