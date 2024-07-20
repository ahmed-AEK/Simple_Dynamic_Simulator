#pragma once

#include "GraphicsLogic/GraphicsLogic.hpp"
#include "toolgui/MouseInteractable.hpp"

namespace node
{
class BlockObject;
class GraphicsScene;
class GraphicsObject;

class NodeDeleteLogic: public GraphicsLogic
{
public:
	explicit NodeDeleteLogic(BlockObject& node, GraphicsScene* scene);
protected:
	void OnMouseMove(const SDL_Point& current_mouse_point) override;
	MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;

private:
	HandlePtr<GraphicsObject> m_object;
};
}