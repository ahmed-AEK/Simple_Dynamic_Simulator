#pragma once

#include "GraphicsScene/GraphicsObject.hpp"
#include "SDL_Framework/SVGRasterizer.hpp"

namespace node
{

class GraphicsObjectsManager;
class BlockObject;

class HoverHighlightObject : public ObjectAttachment
{
public:
	static model::Rect RectForObject(const model::Rect& rect);
	HoverHighlightObject(GraphicsObject& parent_object, GraphicsObjectsManager* manager, const model::ObjectSize& size);

	void OnAttachObject(GraphicsObject& object) override;
	void OnDetachObject() override;
	void OnObjectRectUpdate(const model::Rect& rect) override;

	void Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer) override;
protected:
	GraphicsObject* OnGetInteractableAtPoint(const model::Point& point) override;
private:
	model::Rect GetInnerRect();
	HandlePtr<GraphicsObject> m_parent_object;
	GraphicsObjectsManager* m_manager;
	static constexpr int side_width = 3;
	logging::Logger m_logger = logger(logging::LogCategory::GUI);
};

}