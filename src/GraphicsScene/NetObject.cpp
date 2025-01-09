#include "NetObject.hpp"
#include "BlockSocketObject.hpp"
#include "BlockObject.hpp"
#include <cmath>
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include <cassert>
#include <algorithm>
#include "SDL_Framework/SDLRenderer.hpp"

node::NetSegment::NetSegment(const model::NetSegmentOrientation& orientation, NetNode* startNode, NetNode* endNode)
	: GraphicsObject(model::ObjectSize{ c_width, c_width }, ObjectType::netSegment, nullptr),
	m_startNode(nullptr), m_endNode(nullptr), m_orientation(orientation)
{
	SetAligned(false);
	if (startNode && endNode)
	{
		Connect(startNode, endNode, orientation);
	}
	
}

void node::NetSegment::Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer)
{
	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	SDL_FRect ScreenRect = transformer.SpaceToScreenRect(GetSize().ToRect());
	SDL_RenderFillRect(renderer, &ScreenRect);
}

void node::NetSegment::Connect(NetNode* start, NetNode* end, const model::NetSegmentOrientation& orientation)
{
	assert(start);
	assert(end);
	m_startNode = start;
	m_endNode = end;
	switch(orientation)
	{
		using enum model::NetSegmentOrientation;
		using enum model::ConnectedSegmentSide;
	case vertical:
	{
		m_orientation = vertical;
		if (start->getCenter().y > end->getCenter().y)
		{
			start->setSegment(this, north);
			end->setSegment(this, south);
		}
		else
		{
			start->setSegment(this, south);
			end->setSegment(this, north);
		}
		break;
	}
	case horizontal:
	{
		m_orientation = horizontal;
		if (start->getCenter().x > end->getCenter().x)
		{
			start->setSegment(this, west);
			end->setSegment(this, east);
		}
		else
		{
			start->setSegment(this, east);
			end->setSegment(this, west);
		}
		break;
	}
	}
	this->CalcRect();
}

void node::NetSegment::Disconnect()
{
	m_startNode->ClearSegment(this);
	m_endNode->ClearSegment(this);
}

void node::NetSegment::CalcRect()
{
	if (m_startNode && m_endNode)
	{
		auto rect = model::NetSegmentOrientation::horizontal == m_orientation ?
			model::Rect{ 
				std::min(m_startNode->getCenter().x, m_endNode->getCenter().x),
				std::min(m_startNode->getCenter().y, m_endNode->getCenter().y) - c_width / 2,
				std::abs(m_endNode->getCenter().x - m_startNode->getCenter().x),
				c_width } :
			model::Rect{ 
				std::min(m_startNode->getCenter().x, m_endNode->getCenter().x) - c_width / 2,
				std::min(m_startNode->getCenter().y, m_endNode->getCenter().y),
				c_width, std::abs(m_endNode->getCenter().y - m_startNode->getCenter().y) };
		SetPosition({rect.x, rect.y});
		SetSize({ rect.w, rect.h });
	}
}

node::NetNode::NetNode(const model::Point& center)
	: GraphicsObject({m_width, m_height}, ObjectType::netNode, nullptr)
{
	SetAligned(false);
	setCenter(center);
}

void node::NetNode::Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_FRect screenRect = transformer.SpaceToScreenRect(GetSize().ToRect());
	SDL_RenderFillRect(renderer, &screenRect);
}

void node::NetNode::UpdateConnectedSegments()
{
	for (const auto& segment : m_connected_segments)
	{
		if (segment)
		{
			segment->CalcRect();
		}
	}
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

uint8_t node::NetNode::GetConnectedSegmentsCount() const
{
	auto ret_val = std::count_if(m_connected_segments.begin(), m_connected_segments.end(), 
		[](const auto& segment) { return segment != nullptr; });
	return static_cast<uint8_t>(ret_val);
}

void node::NetNode::ClearSegment(const NetSegment* target_segment)
{
	auto it = std::find_if(m_connected_segments.begin(), m_connected_segments.end(),
		[&](const auto& segment) {return segment == target_segment; });
	if (it != m_connected_segments.end())
	{
		*it = nullptr;
	}
}

std::optional<node::model::ConnectedSegmentSide> node::NetNode::GetSegmentSide(const NetSegment& segment) const
{
	for (size_t i = 0; i < 4; i++)
	{
		if (&segment == m_connected_segments[i])
		{
			return static_cast<node::model::ConnectedSegmentSide>(i);
		}
	}
	return std::nullopt;
}

void node::NetNode::setSegment(NetSegment* segment, model::ConnectedSegmentSide side)
{
	assert(static_cast<size_t>(side) < 4);
	m_connected_segments[static_cast<size_t>(side)] = segment;
}


