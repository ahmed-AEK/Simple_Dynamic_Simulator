#pragma once

#include "GraphicsScene/tools/GraphicsTool.hpp"

namespace node
{

class GraphicsObject;

class ArrowTool: public GraphicsTool
{
public:
	using GraphicsTool::GraphicsTool;
	MI::ClickEvent OnLMBDown(const model::Point& p) override;
	MI::ClickEvent OnLMBUp(const model::Point& p) override;
	void OnMouseMove(const model::Point& p) override;
	bool InternalSelectObject(GraphicsObject* object);
private:
	bool m_isDragging = false;
	SDL_Point m_startPointScreen;
	model::Point m_startEdgeSpace;
};
}