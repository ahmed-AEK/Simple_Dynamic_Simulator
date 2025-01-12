#pragma once

#include "GraphicsScene/GraphicsObject.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"

namespace node
{

class GraphicsObjectsManager;
class BlockObject;

class BlockResizeObject : public GraphicsObject
{
public:
	static model::Rect RectForBlockRect(const model::Rect& rect);
	BlockResizeObject(BlockObject& parent_block, GraphicsObjectsManager* manager, const model::ObjectSize& size);

	void Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer) override;
protected:
	GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;

private:
	model::Rect GetInnerRect();
	HandlePtrS<BlockObject, GraphicsObject> m_parent_block;
	GraphicsObjectsManager* m_manager;
	SVGRasterizer m_rotate_rasterizer;
	static constexpr int corner_width = 15;
};

}