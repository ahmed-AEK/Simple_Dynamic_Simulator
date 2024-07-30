#include "BlockSocketObject.hpp"
#include "BlockObject.hpp"
#include "SDL_Framework/SDL_headers.h"
#include "NetObject.hpp"
#include "IGraphicsScene.hpp"
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include <cassert>

void node::BlockSocketObject::SetConnectedNode(NetNode* node)
{
	if (node == m_connected_node)
	{
		return;
	}
	NetNode* old_node = m_connected_node;
	m_connected_node = node;
	if (old_node)
	{
		old_node->SetConnectedSocket(nullptr);
	}
	if (node)
	{
		node->SetConnectedSocket(this);
	}
}

node::NetNode* node::BlockSocketObject::GetConnectedNode() noexcept
{
	return m_connected_node;
}

node::BlockSocketObject::BlockSocketObject(model::BlockSocketModel::SocketId id,
	model::BlockSocketModel::SocketType type, IGraphicsScene* parentScene, BlockObject* parentNode)
	: GraphicsObject(model::Rect{0,0,nodeLength,nodeLength}, ObjectType::socket, parentScene),
	m_parentNode(parentNode), m_socktType(type), m_id{id}
{
	b_selectable = false;
	b_draggable = false;
}

void node::BlockSocketObject::OnSetSpaceRect(const model::Rect& rect)
{
	GraphicsObject::OnSetSpaceRect(rect);
	if (m_connected_node)
	{
		m_connected_node->setCenter(GetCenter());
		m_connected_node->UpdateConnectedSegments();
	}
}

void node::BlockSocketObject::SetPosition(SDL_Point p)
{
	// set socket position
	SetSpaceRect({ p.x, p.y, nodeLength, nodeLength });
	if (!m_connected_node)
	{
		return;
	}

	// set connected node position
	m_connected_node->setCenter(GetCenter());
	NetSegment* next_segment = m_connected_node->getSegment(NetSide::East);
	if (!next_segment) 
	{
		next_segment = m_connected_node->getSegment(NetSide::West);
	}

	NetNode* next_node = next_segment->getStartNode();
	if (next_node == m_connected_node)
	{
		next_node = next_segment->getEndNode();
	}
	
	// set position of the first node after one segment
	next_node->setCenter({next_node->getCenter().x, GetCenter().y});
	next_node->UpdateConnectedSegments();
}

node::model::Point node::BlockSocketObject::GetCenter()
{
	return { GetSpaceRect().x + nodeLength/2, GetSpaceRect().y + nodeLength / 2};
}

void node::BlockSocketObject::Draw(SDL_Renderer* renderer)
{
	UNUSED_PARAM(renderer);
}

MI::ClickEvent node::BlockSocketObject::OnLMBDown(const model::Point& current_mouse_point)
{
	/*
	if (!m_connected_node)
	{
		std::unique_ptr<NetNode> node1_ptr = std::make_unique<NetNode>(this->GetCenter(), this->GetScene());
		std::unique_ptr<NetNode> node2_ptr = std::make_unique<NetNode>(current_mouse_point, this->GetScene());

		node1_ptr->SetConnectedSocket(this);
		std::unique_ptr<NewNetObject> net_obj = std::make_unique<NewNetObject>(node1_ptr.get(), node2_ptr.get(), GetScene());
		this->GetScene()->SetCurrentHover(net_obj.get());

		this->GetScene()->AddObject(std::move(node1_ptr), NET_NODE_OBJECT_Z);
		this->GetScene()->AddObject(std::move(node2_ptr), NET_NODE_OBJECT_Z);
		this->GetScene()->AddObject(std::move(net_obj), 0);

		return MI::ClickEvent::CAPTURE_START;
	}
	else
	{
		return MI::ClickEvent::CLICKED;
	}
	*/
	return GraphicsObject::LMBDown(current_mouse_point);
}
