#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"

namespace node
{

namespace logic
{

class ScreenDragLogic: public GraphicsLogic
{
public:
	ScreenDragLogic(SDL_Point startPointScreen, model::Point startEdgeSpace, GraphicsScene* scene);
protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	void OnCancel() override;
private:
	SDL_Point m_startPointScreen;
	model::Point m_startEdgeSpace;
};

}
}