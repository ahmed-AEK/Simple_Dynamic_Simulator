#include "VSegmentDragLogic.hpp"
#include "NetObject.hpp"
#include "NodeModels/SceneModelManager.hpp"
#include "GraphicsObjectsManager.hpp"

namespace node
{
	struct SimpleNodeMoveHandler : public logic::NodeDragHandler
	{
		SimpleNodeMoveHandler(const model::Point& start_point, NetNode& node, model::ConnectedSegmentSide segment_side)
			:NodeDragHandler{ start_point, node }, m_segment_side{ segment_side } {}

		void OnMove(const model::Point& new_point) override
		{
			GetBaseNode()->setCenter({ new_point.x - m_start_point.x + m_node_start_position.x , m_node_start_position.y });
		}

		NetNode* GetRepresentingNode() override
		{
			return GetBaseNode();
		}

		AddedNode CreateRequest(NetModificationRequest& request, const model::Point& last_point) override
		{
			assert(GetBaseNode()->GetId());
			request.update_nodes.push_back(NetModificationRequest::UpdateNodeRequest{ *GetBaseNode()->GetId(),
				{ last_point.x - m_start_point.x + m_node_start_position.x , m_node_start_position.y } });
			return { NetModificationRequest::NodeIdType::existing_id, *GetBaseNode()->GetId(), m_segment_side};
		}
		virtual void OnCancel() override
		{
			GetBaseNode()->setCenter(m_node_start_position);
		}

		model::ConnectedSegmentSide m_segment_side;
	};
}
std::unique_ptr<node::logic::VSegmentDragLogic> node::logic::VSegmentDragLogic::Create(NetNode& node1, NetNode& node2, NetSegment& segment, 
	const model::Point& start_point, GraphicsScene* scene, GraphicsObjectsManager* manager)
{
	std::unique_ptr<NodeDragHandler> first_node_handler;	
	std::unique_ptr<NodeDragHandler> second_node_handler;

	std::vector<NetNode*> spare_nodes;
	std::vector<NetSegment*> spare_segments;
	if (node1.GetConnectedSegmentsCount() == 2)
	{
		std::optional<model::ConnectedSegmentSide> side = std::nullopt;
		for (size_t i = 0; i < 4; i++)
		{
			const auto* segment_ptr = node1.getSegment(static_cast<model::ConnectedSegmentSide>(i));
			if (segment_ptr != nullptr && segment_ptr == &segment)
			{
				side = static_cast<model::ConnectedSegmentSide>(i);
			}
		}
		if (!side)
		{
			return nullptr;
		}
		first_node_handler = std::make_unique<SimpleNodeMoveHandler>(start_point, node1, *side);
	}
	else
	{
		return nullptr;
	}

	if (node2.GetConnectedSegmentsCount() == 2)
	{
		std::optional<model::ConnectedSegmentSide> side = std::nullopt;
		for (size_t i = 0; i < 4; i++)
		{
			const auto* segment_ptr = node2.getSegment(static_cast<model::ConnectedSegmentSide>(i));
			if (segment_ptr != nullptr && segment_ptr == &segment)
			{
				side = static_cast<model::ConnectedSegmentSide>(i);
			}
		}
		if (!side)
		{
			return nullptr;
		}
		second_node_handler = std::make_unique<SimpleNodeMoveHandler>(start_point, node2, *side);
	}
	else
	{
		return nullptr;
	}

	auto viewer_node1 = std::make_unique<NetNode>(node1.getCenter());
	auto viewer_node2 = std::make_unique<NetNode>(node2.getCenter());
	auto viewer_segment = std::make_unique<NetSegment>(model::NetSegmentOrientation::vertical, viewer_node1.get(), viewer_node2.get());
	SegmentViewer viewer{ *viewer_node1.get(), *viewer_node2.get(), *viewer_segment.get() };
	spare_nodes.push_back(viewer_node1.get());
	spare_nodes.push_back(viewer_node2.get());
	spare_segments.push_back(viewer_segment.get());

	auto* viewer_node1_ptr = viewer_node1.get();
	auto* viewer_node2_ptr = viewer_node2.get();
	auto* viewer_segment_ptr = viewer_segment.get();

	scene->AddObject(std::move(viewer_node1), GraphicsScene::NetNodeLayer);
	scene->AddObject(std::move(viewer_node2), GraphicsScene::NetNodeLayer);
	scene->AddObject(std::move(viewer_segment), GraphicsScene::SegmentLayer);
	viewer_node1_ptr->SetSelected(true);
	viewer_node2_ptr->SetSelected(true);
	viewer_segment_ptr->SetSelected(true);

	segment.SetVisible(false);
	auto segment_ptr_obj = scene->PopObject(&segment);
	std::unique_ptr<NetSegment> segment_ptr{ static_cast<NetSegment*>(segment_ptr_obj.release()) };
	return std::make_unique<VSegmentDragLogic>(std::move(viewer), std::move(segment_ptr),
		std::move(first_node_handler), std::move(second_node_handler),std::move(spare_nodes), std::move(spare_segments),
		scene, manager);
}

node::logic::VSegmentDragLogic::VSegmentDragLogic(SegmentViewer segment_viewer, std::unique_ptr<NetSegment> segment, std::unique_ptr<NodeDragHandler> first_node_handler, 
	std::unique_ptr<NodeDragHandler> second_node_handler, std::vector<NetNode*> spare_nodes, std::vector<NetSegment*> spare_segments,
	GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_segment_viewer{ std::move(segment_viewer) }, m_first_node_handler{std::move(first_node_handler)},
	m_second_node_handler{std::move(second_node_handler)},  m_base_segment{ std::move(segment)},
	m_spare_nodes{std::move(spare_nodes)}, m_spare_segments{std::move(spare_segments)}
{
}

void node::logic::VSegmentDragLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	m_first_node_handler->OnMove(current_mouse_point);
	m_second_node_handler->OnMove(current_mouse_point);
	m_segment_viewer.Update(m_first_node_handler->GetRepresentingNode()->getCenter(), m_second_node_handler->GetRepresentingNode()->getCenter());
	m_first_node_handler->GetRepresentingNode()->UpdateConnectedSegments();
	m_second_node_handler->GetRepresentingNode()->UpdateConnectedSegments();
}

void node::logic::VSegmentDragLogic::OnCancel()
{
	CleanUp();
}

MI::ClickEvent node::logic::VSegmentDragLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	NetModificationRequest request;
	m_first_node_handler->CreateRequest(request, current_mouse_point);
	m_second_node_handler->CreateRequest(request, current_mouse_point);
	CleanUp();
	GetObjectsManager()->GetSceneModel()->UpdateNet(request);
	return MI::ClickEvent::CAPTURE_END;
}

void node::logic::VSegmentDragLogic::CleanUp()
{
	if (!m_first_node_handler->GetBaseNode())
	{
		return;
	}
	if (!m_second_node_handler->GetBaseNode())
	{
		return;
	}

	m_first_node_handler->OnCancel();
	m_second_node_handler->OnCancel();
	m_base_segment->SetVisible(true);
	auto* base_segment_ptr = m_base_segment.get();
	GetScene()->AddObject(std::move(m_base_segment), GraphicsScene::SegmentLayer);
	AddSelectConnectedNet(*base_segment_ptr, *GetScene());
	m_first_node_handler->GetBaseNode()->UpdateConnectedSegments();
	m_second_node_handler->GetBaseNode()->UpdateConnectedSegments();
	for (const auto* node : m_spare_nodes)
	{
		GetScene()->PopObject(node);
	}
	for (const auto* segment : m_spare_segments)
	{
		GetScene()->PopObject(segment);
	}
}

node::logic::SegmentViewer::SegmentViewer(NetNode& node1, NetNode& node2, NetSegment& segment)
	:m_first_node{node1}, m_second_node{node2}, m_segment{segment}
{
}

void node::logic::SegmentViewer::Update(const model::Point& pos1, const model::Point& pos2)
{
	if (!m_first_node.isAlive() || !m_second_node.isAlive() || !m_segment.isAlive())
	{
		return;
	}
	m_first_node.GetObjectPtr()->setCenter(pos1);
	m_second_node.GetObjectPtr()->setCenter(pos2);
	m_first_node.GetObjectPtr()->UpdateConnectedSegments();
}

node::logic::NodeDragHandler::NodeDragHandler(const model::Point& start_point, NetNode& node)
	:m_start_point{ start_point }, m_node{ node }, m_node_start_position{ node.getCenter() } {}

node::NetNode* node::logic::NodeDragHandler::GetBaseNode()
{ return m_node.GetObjectPtr(); }

