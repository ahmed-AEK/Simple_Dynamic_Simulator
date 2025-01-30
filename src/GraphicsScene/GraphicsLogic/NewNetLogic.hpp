#pragma once


#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "GraphicsScene/GraphicsObject.hpp"
#include "GraphicsScene/GraphicsLogic/TemporaryNetManager.hpp"
#include <array>
#include <variant>

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
	static std::unique_ptr<NewNetLogic> CreateFromSocket(BlockSocketObject& socket, 
		GraphicsScene* scene, GraphicsObjectsManager* manager);
	static std::unique_ptr<NewNetLogic> CreateFromSegment(NetSegment& base_segment, const model::Point& start_point,
		GraphicsScene* scene, GraphicsObjectsManager* manager);
	NewNetLogic(TemporaryNetManager&& net, GraphicsScene* scene, GraphicsObjectsManager* manager);

protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
	void OnCancel() override;

private:
	void ResetNodes();
	void PositionNodes(const model::Point& target_point);
	NetModificationRequest PopulateResultNet(const model::Point& current_mouse_point);
	BlockSocketObject* GetSocketAt(const model::Point& point) const;
	NetSegment* GetSegmentAt(const model::Point& point) const;
	void CleanUp();
	TemporaryNetManager m_net;
};


}

}