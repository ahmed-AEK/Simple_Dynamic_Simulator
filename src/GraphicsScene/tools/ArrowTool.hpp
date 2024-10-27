#pragma once

#include "GraphicsScene/tools/GraphicsTool.hpp"
#include "NodeModels/Observer.hpp"

namespace node
{
class BlockObject;
class BlockResizeObject;

struct BlockDoubleClickedEvent
{
	BlockObject* block;
};

class GraphicsObject;

class ArrowTool: public GraphicsTool, public SinglePublisher<BlockDoubleClickedEvent>
{
public:
	using GraphicsTool::GraphicsTool;
	MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
	MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
	void OnMouseMove(MouseHoverEvent& e) override;
	bool InternalSelectObject(GraphicsObject* object);
	void OnExit() override; // when tool is changed
private:
	std::unique_ptr<BlockResizeObject> CreateResizeObject(BlockObject& block);

	HandlePtr<GraphicsObject> m_last_clicked_block = nullptr;
	HandlePtr<GraphicsObject> m_current_block_resize_object;
};
}