#pragma once


#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "GraphicsScene/GraphicsObject.hpp"
#include <array>

namespace node {

class NetNode;
class NetSegment;
class GraphicsScene;
class BlockSocketObject;
struct NetModificationRequest;
namespace model
{
	class NetModel;
	using NetModelPtr = std::shared_ptr<NetModel>;
}

namespace logic
{

class GRAPHICSSCENE_API NewNetLogic : public node::logic::GraphicsLogic
{
public:
	static std::unique_ptr<NewNetLogic> Create(BlockSocketObject* socket, 
		GraphicsScene* scene, GraphicsObjectsManager* manager);
	NewNetLogic(BlockSocketObject* socket, std::array<NetNode*, 4> nodes,	
		std::array<NetSegment*, 3> segments, GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
	void OnCancel() override;

private:
	NetModificationRequest PopulateResultNet(const model::Point& current_mouse_point);
	BlockSocketObject* GetSocketAt(const model::Point& point) const;
	void DeleteAllOwnedObjects();
	std::array<HandlePtr<GraphicsObject>, 4> m_nodes;
	std::array<HandlePtr<GraphicsObject>, 3> m_segments;
	HandlePtr<GraphicsObject> m_socket;
};


}

}