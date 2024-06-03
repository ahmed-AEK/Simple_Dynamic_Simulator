#include "NetObject.hpp"
#include "NodeSocket.hpp"
#include "Node.hpp"
#include "GraphicsLogic/NewNet.hpp"
#include "GraphicsLogic/NewNetJunction.hpp"
#include "IGraphicsScene.hpp"
#include "IGraphicsSceneController.hpp"
#include <cmath>

node::NetSegment::NetSegment(const NetOrientation& orientation, NetNode* startNode, NetNode* endNode, node::IGraphicsScene* scene)
	: GraphicsObject({0,0,0,0}, ObjectType::net, scene),
	m_startNode(nullptr), m_endNode(nullptr), m_orientation(orientation)
{
	b_draggable = false;
	b_aligned = false;
	if (startNode && endNode)
	{
		Connect(startNode, endNode, orientation);
	}
	
}

void node::NetSegment::Draw(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	SDL_RenderFillRect(renderer, &(this->GetRectImpl()));
}

MI::ClickEvent node::NetSegment::OnLMBDown(const SDL_Point& current_mouse_point)
{
	/*
	UNUSED_PARAM(current_mouse_point);
	if (GraphicsSceneMode::Insert == GetScene()->GetMode())
	{
		if (NetOrientation::Vertical == GetOrientation() && node::NewNetJunctionObject::TryCreate(this, current_mouse_point, GetScene()))
		{
			return MI::ClickEvent::CAPTURE_START;
		}
	}
	else if (GraphicsSceneMode::Delete == GetScene()->GetMode())
	{
		b_being_deleted = true;
		GetScene()->SetCurrentHover(this);
		return MI::ClickEvent::CAPTURE_START;
	}
	else if (!m_startNode->GetConnectedSocket() && !m_endNode->GetConnectedSocket() && 
		m_startNode->GetConnectedSegmentsCount() < 3 && m_endNode->GetConnectedSegmentsCount() < 3)
	{
		b_being_dragged = true;
		GetScene()->SetCurrentHover(this);
		return MI::ClickEvent::CAPTURE_START;
	}
	return MI::ClickEvent::NONE;
	*/
	auto&& scene = GetScene();
	if (!scene)
	{
		return MI::ClickEvent::NONE;
	}
	auto&& controller = scene->GetController();
	if (!controller)
	{
		return MI::ClickEvent::NONE;
	}

	return controller->OnSegmentLMBDown(current_mouse_point, *this);
}
void node::NetSegment::OnMouseMove(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	/*
	if (b_being_dragged)
	{
		switch(m_orientation)
		{
			case NetOrientation::Vertical:
				m_startNode->setCenter({current_mouse_point.x, m_startNode->getCenter().y});
				m_endNode->setCenter({current_mouse_point.x, m_endNode->getCenter().y});
				break;
			case NetOrientation::Horizontal:
				m_startNode->setCenter({m_startNode->getCenter().x, current_mouse_point.y});
				m_endNode->setCenter({m_endNode->getCenter().x, current_mouse_point.y});
				break;
		}
		m_startNode->UpdateConnectedSegments();
		m_endNode->UpdateConnectedSegments();
	}
	if (b_being_deleted && !SDL_PointInRect(&current_mouse_point, &GetSpaceRect()))
	{
		b_being_deleted = false;
	}
	*/
}
MI::ClickEvent node::NetSegment::OnLMBUp(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	/*
	UNUSED_PARAM(current_mouse_point);
	if (b_being_dragged)
	{
		b_being_dragged = false;
		return MI::ClickEvent::CAPTURE_END;
	}
	if (GraphicsSceneMode::Delete == GetScene()->GetMode() && b_being_deleted)
	{
		std::unique_ptr<GraphicsObject> this_ptr = GetScene()->PopObject(this);
		UNUSED_PARAM(this_ptr);
		NetNode* start_node = m_startNode;
		m_startNode->ClearSegment(this);
		if (0 == start_node->GetConnectedSegmentsCount())
		{
			if (start_node->GetConnectedSocket())
			{
				start_node->SetConnectedSocket(nullptr);
			}
			std::unique_ptr<GraphicsObject> unique_start_node = GetScene()->PopObject(start_node);
			UNUSED_PARAM(unique_start_node);
		}
		NetNode* end_node = m_endNode;
		m_endNode->ClearSegment(this);
		if (0 == end_node->GetConnectedSegmentsCount())
		{
			if (end_node->GetConnectedSocket())
			{
				end_node->SetConnectedSocket(nullptr);
			}
			std::unique_ptr<GraphicsObject> unique_end_node = GetScene()->PopObject(end_node);
			UNUSED_PARAM(unique_end_node);
		}
		GetScene()->SetCurrentHover(nullptr);
		return MI::ClickEvent::CAPTURE_END;
	}
	return MI::ClickEvent::NONE;
	*/
	return MI::ClickEvent::NONE;
}

void node::NetSegment::Connect(NetNode* start, NetNode* end, const NetOrientation& orientation)
{
	m_startNode = start;
	m_endNode = end;
	switch(orientation)
	{
	case NetOrientation::Vertical:
	{
		m_orientation = NetOrientation::Vertical;
		if (start->getCenter().y > end->getCenter().y)
		{
			start->setSegment(this, NetSide::North);
			end->setSegment(this, NetSide::South);
		}
		else
		{
			start->setSegment(this, NetSide::South);
			end->setSegment(this, NetSide::North);
		}
		break;
	}
	case NetOrientation::Horizontal:
	{
		m_orientation = NetOrientation::Horizontal;
		if (start->getCenter().x > end->getCenter().x)
		{
			start->setSegment(this, NetSide::West);
			end->setSegment(this, NetSide::East);
		}
		else
		{
			start->setSegment(this, NetSide::East);
			end->setSegment(this, NetSide::West);
		}
		break;
	}
	}
	this->CalcRect();
}

void node::NetSegment::Disconnect()
{
	auto remove_self = [&](NetNode*& node)
	{
	if (node)
	{
		if (NetOrientation::Vertical == m_orientation)
		{
			if (node->getSegment(NetSide::North) == this)
			{
				node->setSegment(nullptr, NetSide::North);
			}
			else if (node->getSegment(NetSide::South) == this)
			{
				node->setSegment(nullptr, NetSide::South);
			}
		}
		else
		{
			if (node->getSegment(NetSide::East) == this)
			{
				node->setSegment(nullptr, NetSide::East);
			}
			else if (node->getSegment(NetSide::West) == this)
			{
				node->setSegment(nullptr, NetSide::West);
			}
		}
		node = nullptr;
	}
	};
	
	remove_self(m_startNode);
	remove_self(m_endNode);
}

void node::NetSegment::CalcRect()
{
	if (m_startNode && m_endNode)
	{
		SetSpaceRect(NetOrientation::Horizontal == m_orientation ?
			SDL_Rect{ std::min(m_startNode->getCenter().x, m_endNode->getCenter().x),
			std::min(m_startNode->getCenter().y, m_endNode->getCenter().y) - c_width/2,
			std::abs(m_endNode->getCenter().x - m_startNode->getCenter().x),c_width} :
			SDL_Rect{ std::min(m_startNode->getCenter().x, m_endNode->getCenter().x) - c_width/2, 
			std::min(m_startNode->getCenter().y, m_endNode->getCenter().y),
			c_width, std::abs(m_endNode->getCenter().y - m_startNode->getCenter().y) });
	}
}

node::NetNode::NetNode(const SDL_Point& center, IGraphicsScene* scene)
	: GraphicsObject({center.x - m_width/2, center.y - m_height/2, m_width, m_height}, ObjectType::netNode, scene), m_centerPoint(center)
{
	b_draggable = false;
	b_aligned = false;
}

void node::NetNode::Draw(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &(this->GetRectImpl()));
}

void node::NetNode::UpdateConnectedSegments()
{
	if (m_northSegment) { m_northSegment->CalcRect(); }
	if (m_southSegment) { m_southSegment->CalcRect(); }
	if (m_eastSegment) { m_eastSegment->CalcRect(); }
	if (m_westSegment) { m_westSegment->CalcRect(); }
}
void node::NetNode::SetConnectedSocket(NodeSocket* socket)
{
	if (socket == m_socket)
	{
		return;
	}
	NodeSocket* old_socket = m_socket;
	this->m_socket = socket;
	if (old_socket)
	{
		old_socket->SetConnectedNode(nullptr);
	}
	if (socket)
	{
		socket->SetConnectedNode(this);
	}
}

node::NodeSocket* node::NetNode::GetConnectedSocket() noexcept
{
	return m_socket;
}

uint8_t node::NetNode::GetConnectedSegmentsCount()
{
	uint8_t ret_val = 0;
	ret_val += m_northSegment != 0;
	ret_val += m_southSegment != 0;
	ret_val += m_eastSegment != 0;
	ret_val += m_westSegment != 0;
	return ret_val;
}

void node::NetNode::ClearSegment(const NetSegment* segment)
{

	auto clearSegment = [&](NetSegment*& target_segment)
	{
		if (target_segment == segment)
		{
			target_segment = nullptr;
		}
	};
	clearSegment(m_northSegment);
	clearSegment(m_southSegment);
	clearSegment(m_westSegment);
	clearSegment(m_eastSegment);
}

MI::ClickEvent node::NetNode::OnLMBDown(const SDL_Point& current_mouse_point)
{
	/*
	UNUSED_PARAM(current_mouse_point);
	if (GraphicsSceneMode::Delete == GetScene()->GetMode())
	{
			b_being_deleted = true;
			GetScene()->SetCurrentHover(this);
			return MI::ClickEvent::CAPTURE_START;
	}
	if (!m_socket && 1 == GetConnectedSegmentsCount())
	{
		if (node::NewNetObject::TryCreate(this, GetScene()))
		{
			return MI::ClickEvent::CAPTURE_START;
		}
	}
	return MI::ClickEvent::CLICKED;
	*/
	auto&& scene = GetScene();
	if (!scene)
	{
		return MI::ClickEvent::NONE;
	}
	auto&& controller = scene->GetController();
	if (!controller)
	{
		return MI::ClickEvent::NONE;
	}

	return controller->OnNetNodeLMBDown(current_mouse_point, *this);
}

MI::ClickEvent node::NetNode::OnLMBUp(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	/*
	if (GraphicsSceneMode::Delete == GetScene()->GetMode() && b_being_deleted)
	{
		std::array<NetSegment*, 4> segments{ m_northSegment, m_southSegment, m_eastSegment, m_westSegment };
		for (auto segment : segments)
		{
			if (segment)
			{
				NetNode* otherNode = segment->getStartNode();
				if (otherNode == this)
				{
					otherNode = segment->getEndNode();
				}
				segment->Disconnect();
				GetScene()->PopObject(segment);
				if (!otherNode->GetConnectedSegmentsCount())
				{
					otherNode->SetConnectedSocket(nullptr);
					GetScene()->PopObject(otherNode);
				}
			}
		}
		SetConnectedSocket(nullptr);
		std::unique_ptr<GraphicsObject> this_ptr = GetScene()->PopObject(this);
		UNUSED_PARAM(this_ptr);
		GetScene()->SetCurrentHover(nullptr);
		return MI::ClickEvent::CAPTURE_END;
	}
	return GraphicsObject::OnLMBUp(current_mouse_point);
	*/
	return MI::ClickEvent::NONE;
}

void node::NetNode::OnMouseMove(const SDL_Point& current_mouse_point)
{
	if (!SDL_PointInRect(&current_mouse_point, &GetSpaceRect()))
	{
		b_being_deleted = false;
	}
}

void node::NetNode::OnSetSpaceRect(const SDL_Rect& rect)
{
	m_centerPoint = { rect.x + rect.w/2, rect.y + rect.h/2 };
}

void node::NetNode::setSegment(NetSegment* segment, NetSide side)
{
	switch (side)
	{
	case NetSide::North:
	{
		m_northSegment = segment;
		break;
	}
	case NetSide::South:
	{
		m_southSegment = segment;
		break;
	}
	case NetSide::East:
	{
		m_eastSegment = segment;
		break;
	}
	case NetSide::West:
	{
		m_westSegment = segment;
		break;
	}
	}
}


