#include "NetObject.hpp"
#include "BlockSocketObject.hpp"
#include "BlockObject.hpp"
#include "IGraphicsScene.hpp"
#include <cmath>
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include <cassert>

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
	assert(GetScene());
	SDL_Rect ScreenRect = GetScene()->GetSpaceScreenTransformer().SpaceToScreenRect(GetSpaceRect());
	SDL_RenderFillRect(renderer, &ScreenRect);
}

void node::NetSegment::Connect(NetNode* start, NetNode* end, const NetOrientation& orientation)
{
	assert(start);
	assert(end);
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
			model::Rect{ std::min(m_startNode->getCenter().x, m_endNode->getCenter().x),
			std::min(m_startNode->getCenter().y, m_endNode->getCenter().y) - c_width/2,
			std::abs(m_endNode->getCenter().x - m_startNode->getCenter().x),c_width} :
			model::Rect{ std::min(m_startNode->getCenter().x, m_endNode->getCenter().x) - c_width/2, 
			std::min(m_startNode->getCenter().y, m_endNode->getCenter().y),
			c_width, std::abs(m_endNode->getCenter().y - m_startNode->getCenter().y) });
	}
}

node::NetNode::NetNode(const model::Point& center, IGraphicsScene* scene)
	: GraphicsObject({center.x - m_width/2, center.y - m_height/2, m_width, m_height}, ObjectType::netNode, scene), m_centerPoint(center)
{
	b_draggable = false;
	b_aligned = false;
}

void node::NetNode::Draw(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	assert(GetScene());
	SDL_Rect screenRect = GetScene()->GetSpaceScreenTransformer().SpaceToScreenRect(GetSpaceRect());
	SDL_RenderFillRect(renderer, &screenRect);
}

void node::NetNode::UpdateConnectedSegments()
{
	if (m_northSegment) { m_northSegment->CalcRect(); }
	if (m_southSegment) { m_southSegment->CalcRect(); }
	if (m_eastSegment) { m_eastSegment->CalcRect(); }
	if (m_westSegment) { m_westSegment->CalcRect(); }
}
void node::NetNode::SetConnectedSocket(BlockSocketObject* socket)
{
	if (socket == m_socket)
	{
		return;
	}
	BlockSocketObject* old_socket = m_socket;
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

node::BlockSocketObject* node::NetNode::GetConnectedSocket() noexcept
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

void node::NetNode::OnSetSpaceRect(const model::Rect& rect)
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


