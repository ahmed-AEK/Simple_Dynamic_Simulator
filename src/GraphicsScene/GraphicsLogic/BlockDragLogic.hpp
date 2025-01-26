#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "GraphicsScene/GraphicsLogic/TemporaryNetManager.hpp"
#include "NodeModels/IdTypes.hpp"

#include <unordered_map>

namespace node
{
class BlockObject;
class GraphicsObject;
struct NetModificationRequest;

namespace logic
{

class BlockDragLogic: public GraphicsLogic
{
public:
	static std::unique_ptr<BlockDragLogic> TryCreate(model::Point startPointMouseSpace,
		BlockObject& block, GraphicsScene* scene, GraphicsObjectsManager* manager);

	BlockDragLogic(model::Point startPointMouseSpace, model::Point startObjectEdge, 
		BlockObject& block, std::unordered_map<model::SocketId, TemporaryNetManager> socket_nets,
		GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	void OnCancel() override;
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
private:
	void ResetNodes();
	void CleanUp();
	void PositionNodes(const model::Point& target_point);
	NetModificationRequest PopulateResultNet(const model::Point& current_block_point);
	
	model::Point m_startPointMouseSpace;
	model::Point m_startObjectEdge;
	node::HandlePtrS<BlockObject, GraphicsObject> m_block;
	std::unordered_map<model::SocketId, TemporaryNetManager> m_socket_nets;
	
};

}
}