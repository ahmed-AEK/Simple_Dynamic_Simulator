#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"

namespace node
{

class GraphicsObject;
class NetNode;

namespace logic
{

class NetNodeDeleteLogic: public GraphicsLogic
{
public:
	NetNodeDeleteLogic(NetNode& node, GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	virtual void OnMouseMove(const model::Point& current_mouse_point);
	virtual MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point);
private:
	HandlePtr<GraphicsObject> m_node;
};

}
}