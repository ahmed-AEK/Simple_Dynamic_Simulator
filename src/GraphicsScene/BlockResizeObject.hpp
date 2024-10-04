#pragma once

#include "GraphicsScene/GraphicsObject.hpp"

namespace node
{


class BlockResizeObject : public GraphicsObject
{
public:
	BlockResizeObject(HandlePtr<GraphicsObject> parent_block,model::Rect sceneRect, IGraphicsScene* scene);

	void Draw(SDL_Renderer* renderer) override;
protected:
	GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;

private:
	HandlePtr<GraphicsObject> m_parent_block;
};

}