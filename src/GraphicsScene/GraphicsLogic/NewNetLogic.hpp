#pragma once


#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "GraphicsScene/GraphicsObject.hpp"
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

	struct SocketAnchor
	{
		HandlePtrS<BlockSocketObject, GraphicsObject> socket;
		model::Point position;
	};

	struct SegmentAnchor
	{
		HandlePtrS<NetSegment, GraphicsObject> segment;
		model::Point position;
	};

	using anchor_t = typename std::variant<SocketAnchor, SegmentAnchor>;

	static std::unique_ptr<NewNetLogic> CreateFromSocket(BlockSocketObject& socket, 
		GraphicsScene* scene, GraphicsObjectsManager* manager);
	static std::unique_ptr<NewNetLogic> CreateFromSegment(NetSegment& base_segment, const model::Point& start_point,
		GraphicsScene* scene, GraphicsObjectsManager* manager);
	NewNetLogic(anchor_t start_anchor, std::array<NetNode*, 6> nodes,
		std::array<NetSegment*, 5> segments, GraphicsScene* scene, GraphicsObjectsManager* manager);

protected:
	void OnMouseMove(const model::Point& current_mouse_point) override;
	MI::ClickEvent OnLMBUp(const model::Point& current_mouse_point) override;
	void OnCancel() override;

private:
	void ResetNodes();

	NetModificationRequest PopulateResultNet(const model::Point& current_mouse_point);
	BlockSocketObject* GetSocketAt(const model::Point& point) const;
	void CleanUp();
	std::array<HandlePtrS<NetNode,GraphicsObject>, 6> m_nodes;
	std::array<HandlePtrS<NetSegment, GraphicsObject>, 5> m_segments;
	anchor_t m_start_anchor;
};


}

}