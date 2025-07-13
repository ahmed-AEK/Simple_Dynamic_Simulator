#pragma once

#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "GraphicsScene/GraphicsLogic/TemporaryNetManager.hpp"

#include <variant>
#include <array>

namespace node
{
class NetNode;
class BlockSocketObject;
class GraphicsObject;
class NetSegment;
struct NetModificationRequest;

namespace logic
{

class LeafNetNodeDragLogic : public GraphicsLogic
{
public:
	static std::unique_ptr<LeafNetNodeDragLogic> TryCreate(NetNode& node,
		GraphicsScene& scene, GraphicsObjectsManager& manager);
	LeafNetNodeDragLogic(TemporaryNetManager net_manager,
		GraphicsScene* scene, GraphicsObjectsManager* manager);
protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
	void OnCancel() override;
private:
	BlockSocketObject* GetSocketAt(const model::Point& point) const;
	NetSegment* GetSegmentAt(const model::Point& point) const;

	void AddNetModificationsWithSegment(NetModificationRequest& request, const NetSegment& end_segment) const;
	void AddNetModificationsWithSocket(NetModificationRequest& request, const BlockSocketObject* end_socket) const;

	void ResetNodes();
	void PositionNodes(const model::Point& target_point);
	void CleanUp();
	NetModificationRequest PopulateResultNet(const model::Point& current_mouse_point);

	TemporaryNetManager m_temp_net_mgr;
};
}
}