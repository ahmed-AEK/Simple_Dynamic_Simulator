#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"

namespace node
{

namespace logic
{

class ScreenDragLogic: public GraphicsLogic
{
public:
	ScreenDragLogic(SDL_FPoint startPointScreen, model::Point startEdgeSpace, 
		GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	void OnCancel() override;
private:
	SDL_FPoint m_startPointScreen;
	model::Point m_startEdgeSpace;
};

}
}