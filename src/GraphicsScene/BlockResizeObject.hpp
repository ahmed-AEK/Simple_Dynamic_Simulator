#pragma once

#include "GraphicsScene/GraphicsObject.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"

namespace node
{

class GraphicsObjectsManager;
class BlockObject;

class BlockResizeObject : public ObjectAttachment
{
public:
	static model::Rect RectForBlockRect(const model::Rect& rect);
	BlockResizeObject(BlockObject& parent_block, GraphicsObjectsManager* manager, const model::ObjectSize& size);

	void OnAttachObject(GraphicsObject& object) override;
	void OnDetachObject() override;
	void OnObjectRectUpdate(const model::Rect& rect) override;

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
	logging::Logger m_logger = logger(logging::LogCategory::GUI);
};

}