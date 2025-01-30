#include "NetObject.hpp"
#include "BlockSocketObject.hpp"
#include "BlockObject.hpp"
#include "GraphicsScene/GraphicsScene.hpp"

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
	if (IsSelected())
	{
		SDL_SetRenderDrawColor(renderer, 255, 180, 0, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	}
	SDL_FRect ScreenRect = transformer.SpaceToScreenRect(GetSize().ToRect());
	SDL_RenderFillRect(renderer, &ScreenRect);
}

void node::NetSegment::Connect(NetNode* start, NetNode* end, const model::NetSegmentOrientation& orientation)
{
	assert(start);
	assert(end);

	Disconnect();

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
	if (m_startNode)
	{
		assert(m_endNode);
		m_startNode->ClearSegment(this);
		m_endNode->ClearSegment(this);
	}
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

node::model::Point node::NetSegment::GetCenter() const
{
	auto rect = GetSceneRect();
	return {rect.x + rect.w / 2, rect.y + rect.h / 2};
}

node::NetNode::NetNode(const model::Point& center)
	: GraphicsObject({m_width, m_height}, ObjectType::netNode, nullptr)
{
	SetAligned(false);
	setCenter(center);
}

void node::NetNode::Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer)
{
	if (IsSelected())
	{
		SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	}
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

void node::AddSelectConnectedNet(NetSegment& segment, GraphicsScene& scene)
{
	if (segment.IsSelected())
	{
		return;
	}
	scene.AddSelection(segment.GetMIHandlePtr());
	std::stack<NetNode*> nodes_to_visit;
	if (!segment.getStartNode()->IsSelected())
	{
		nodes_to_visit.push(segment.getStartNode());
	}
	if (!segment.getEndNode()->IsSelected())
	{
		nodes_to_visit.push(segment.getEndNode());
	}
	nodes_to_visit.push(segment.getEndNode());
	while (nodes_to_visit.size())
	{
		auto* node = nodes_to_visit.top();
		nodes_to_visit.pop();
		if (node->GetConnectedSegmentsCount() > 2)
		{
			continue;
		}
		scene.AddSelection(node->GetMIHandlePtr());
		for (int i = 0; i < 4; i++)
		{
			auto* segment_ptr = node->getSegment(static_cast<model::ConnectedSegmentSide>(i));
			if (segment_ptr && !segment_ptr->IsSelected())
			{
				scene.AddSelection(segment_ptr->GetMIHandlePtr());
				if (!segment_ptr->getStartNode()->IsSelected())
				{
					nodes_to_visit.push(segment_ptr->getStartNode());
				}
				if (!segment_ptr->getEndNode()->IsSelected())
				{
					nodes_to_visit.push(segment_ptr->getEndNode());
				}
			}
		}
	}
	
}

void node::AddSelectConnectedNet(NetNode& node, GraphicsScene& scene)
{
	if (node.IsSelected())
	{
		return;
	}
	scene.AddSelection(node.GetMIHandlePtr());
	for (int i = 0; i < 4; i++)
	{
		auto* segment = node.getSegment(static_cast<model::ConnectedSegmentSide>(i));
		if (segment)
		{
			AddSelectConnectedNet(*segment, scene);
		}
	}
}
