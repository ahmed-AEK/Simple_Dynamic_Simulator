#pragma once

#include "GraphicsScene/tools/GraphicsTool.hpp"
#include "NodeModels/Observer.hpp"

namespace node
{
class BlockObject;

struct BlockDoubleClickedEvent
{
	BlockObject* block;
};

class GraphicsObject;

class ArrowTool: public GraphicsTool, public SinglePublisher<BlockDoubleClickedEvent>
{
public:
	using GraphicsTool::GraphicsTool;
	MI::ClickEvent OnLMBDown(const model::Point& p) override;
	MI::ClickEvent OnLMBUp(const model::Point& p) override;
	void OnMouseMove(const model::Point& p) override;
	bool InternalSelectObject(GraphicsObject* object);
	void OnExit() override; // when tool is changed
private:
	int64_t m_last_click_point = 0;
	HandlePtr<GraphicsObject> m_last_clicked_block = nullptr;
	bool b_second_click_in_progress = false;
	
	HandlePtr<GraphicsObject> m_current_block_resize_object;
};
}