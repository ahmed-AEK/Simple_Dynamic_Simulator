#pragma once

#include "GraphicsScene/GraphicsObject.hpp"

namespace node
{

class GraphicsObjectsManager;

class BlockResizeObject : public GraphicsObject
{
public:
	static model::Rect RectForBlockRect(const model::Rect& rect);
	BlockResizeObject(HandlePtr<GraphicsObject> parent_block, GraphicsObjectsManager* manager, model::Rect sceneRect, GraphicsScene* scene = nullptr);

	void Draw(SDL_Renderer* renderer, const SpaceScreenTransformer& transformer) override;
protected:
	GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;
	MI::ClickEvent OnLMBDown(const model::Point& current_mouse_point) override;

private:
	HandlePtr<GraphicsObject> m_parent_block;
	GraphicsObjectsManager* m_manager;
};

}