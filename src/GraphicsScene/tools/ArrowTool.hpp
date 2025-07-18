#pragma once

#include "GraphicsScene/tools/GraphicsTool.hpp"
#include "NodeModels/Observer.hpp"

namespace node
{
class BlockObject;
class BlockResizeObject;
class Widget;

struct BlockDoubleClickedEvent
{
	BlockObject* block;
};

class GraphicsObject;

class ArrowTool: public GraphicsTool, public SinglePublisher<BlockDoubleClickedEvent>
{
public:
	using GraphicsTool::GraphicsTool;
	MI::ClickEvent OnObjectLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager, GraphicsObject& object) override;
	MI::ClickEvent OnSpaceLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager) override;
	bool IsObjectClickable(GraphicsScene& scene, GraphicsObjectsManager& manager, GraphicsObject& object) override;

	bool InternalSelectObject(GraphicsObject* object, GraphicsScene& scene);
	void OnExit() override; // when tool is changed
private:
	std::unique_ptr<BlockResizeObject> CreateResizeObject(BlockObject& block, GraphicsObjectsManager& manager);

	HandlePtr<GraphicsObject> m_last_clicked_block = nullptr;

	struct ResizerObjectSlot
	{
		HandlePtrS<GraphicsScene, Widget> scene;
		HandlePtr<GraphicsObject> object;
	};
	ResizerObjectSlot m_current_block_resize_slot;
	logging::Logger m_logger = logger(logging::LogCategory::GUI);
};
}