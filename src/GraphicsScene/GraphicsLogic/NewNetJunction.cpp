#include "NewNet.hpp"
#include "NewNetJunction.hpp"
#include "GraphicsScene.hpp"
#include "NetObject.hpp"
#include <cassert>

node::NewNetJunctionObject* node::NewNetJunctionObject::TryCreate(NetSegment* startSegment,const SDL_Point& startPoint, GraphicsScene* scene)
{
	assert(startSegment);
	assert(NetOrientation::Vertical == startSegment->GetOrientation());
	assert(scene);
	
	std::unique_ptr<NetNode> start_node = std::make_unique<NetNode>(
		SDL_Point{startSegment->GetSpaceRect().x + startSegment->GetWidth() / 2, startPoint.y }, scene);
	NetNode* start = start_node.get();
	scene->AddObject(std::move(start_node), NET_NODE_OBJECT_Z);
	std::unique_ptr<NetNode> end_node = std::make_unique<NetNode>(startPoint, scene);
	NetNode* end = end_node.get();
	scene->AddObject(std::move(end_node), NET_NODE_OBJECT_Z);
	
	std::unique_ptr<NewNetJunctionObject> net_ptr = std::make_unique<NewNetJunctionObject>(start, end, scene, startSegment);
	NewNetJunctionObject* ptr = net_ptr.get();
	scene->SetGraphicsLogic(std::move(net_ptr));
	return ptr;
}

node::NewNetJunctionObject::NewNetJunctionObject(NetNode* startNode, NetNode* endNode, 
	GraphicsScene* scene, NetSegment* segment_to_fuse)
	:NewNetObject(startNode, endNode, scene), m_segmentToFuse(segment_to_fuse)
{
}


MI::ClickEvent node::NewNetJunctionObject::OnLMBUp(const SDL_Point& current_mouse_point)
{
	if (m_segmentToFuse)
	{
		NetNode* start_node = m_segmentToFuse->getStartNode();
		NetNode* end_node = m_segmentToFuse->getEndNode();
		if (start_node->getCenter().y > end_node->getCenter().y)
		{
			std::swap(start_node, end_node);
		}
		m_segmentToFuse->Disconnect();
		m_segmentToFuse->Connect(start_node, GetStartNode(), NetOrientation::Vertical);
		std::unique_ptr<NetSegment> segment = std::make_unique<NetSegment>(NetOrientation::Vertical,
			GetStartNode(), end_node, GetScene());
		GetScene()->AddObject(std::move(segment), NET_SEGMENT_OBJECT_Z);
		
	}
	return NewNetObject::OnLMBUp(current_mouse_point);
}
