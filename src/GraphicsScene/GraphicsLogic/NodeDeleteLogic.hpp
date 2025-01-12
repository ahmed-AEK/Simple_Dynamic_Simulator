#pragma once

#include "GraphicsLogic/GraphicsLogic.hpp"

namespace node
{
class BlockObject;
class GraphicsScene;
class GraphicsObject;

class NodeDeleteLogic: public logic::GraphicsLogic
{
public:
	explicit NodeDeleteLogic(BlockObject& node, GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;

private:
	HandlePtrS<BlockObject,GraphicsObject> m_object;
};
}