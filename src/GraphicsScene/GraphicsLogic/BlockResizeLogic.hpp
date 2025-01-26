#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "GraphicsScene/GraphicsObject.hpp"
#include "NodeModels/BlockModel.hpp"
#include "GraphicsScene/GraphicsLogic/TemporaryNetManager.hpp"

namespace node
{
class BlockObject;
class BlockResizeObject;

namespace logic
{

class BlockResizeLogic : public GraphicsLogic
{
public:
	enum class DragSide
	{
		LeftTop,
		RightTop,
		LeftBottom,
		RightBottom,
	};
	static std::unique_ptr<BlockResizeLogic> TryCreate(BlockObject& block, BlockResizeObject& resize_object, model::Point drag_start_point,
		DragSide side, GraphicsScene* scene, GraphicsObjectsManager* manager);
	BlockResizeLogic(BlockObject& block, BlockResizeObject& resize_object, model::Point drag_start_point, 
		DragSide side, GraphicsScene* scene, GraphicsObjectsManager* manager, 
		std::unordered_map<model::SocketId, TemporaryNetManager>&& socket_nets);
protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	void OnCancel() override;
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
private:
	void CleanUp();
	void PositionNodes(const model::Point& edge, std::span<const model::BlockSocketModel> sockets);

	std::vector<model::BlockSocketModel> m_temp_sockets;
	HandlePtrS<BlockObject, GraphicsObject> m_block;
	HandlePtrS<BlockResizeObject, GraphicsObject> m_resizeObject;
	model::Rect m_initial_rect;
	model::Point m_drag_start_point;
	DragSide m_drag_side;
	std::unordered_map<model::SocketId, TemporaryNetManager> m_socket_nets;
	static constexpr int min_dim = 40;
};

}

}