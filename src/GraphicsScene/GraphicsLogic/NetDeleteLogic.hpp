#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"

namespace node
{

class GraphicsObject;
class NetSegment;

namespace logic
{

class NetDeleteLogic: public GraphicsLogic
{
public:
	NetDeleteLogic(GraphicsObject& object, GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	virtual void OnMouseMove(const model::Point& current_mouse_point) override;
	virtual MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
private:
	HandlePtr<GraphicsObject> m_object;
};


}
namespace NetUtils
{
	std::optional<NetModificationRequest> GetDeletionRequestForNet(std::span<node::HandlePtr<node::GraphicsObject>> objects);
}
}