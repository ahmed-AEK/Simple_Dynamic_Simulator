#include "NewNet.hpp"

node::NewNetObject* node::NewNetObject::TryCreate(NetNode* endNode, GraphicsScene* scene)
{
	constexpr auto find_next_node = [](NetSegment* segment, NetNode* node)
		{
			NetSegment* other_segment;
			for (int i = 0; i < 4; i++)
			{
				other_segment = node->getSegment(static_cast<NetSide>(i));
				if (other_segment && other_segment != segment)
				{
					break;
				}
			}
			NetNode* other_node = other_segment->getStartNode();
			if (other_node == node)
			{
				other_node = other_segment->getEndNode();
			}
			return std::pair{ other_segment, other_node };
		};
	std::array<NetNode*, 4> nodes{};
	std::array<NetSegment*, 3> segments{};
	if (!endNode || endNode->GetConnectedSegmentsCount() != 1)
	{
		return nullptr;
	}
	nodes[3] = endNode;
	auto&& [segment2, node2] = find_next_node(nullptr, endNode);
	nodes[2] = node2;
	segments[2] = segment2;
	if (node2->GetConnectedSegmentsCount() != 2)
	{
		return nullptr;
	}
	auto&& [segment1, node1] = find_next_node(segment2, node2);
	nodes[1] = node1;
	segments[1] = segment1;
	if (node1->GetConnectedSegmentsCount() != 2)
	{
		return nullptr;
	}
	auto&& [segment0, node0] = find_next_node(segment1, node1);
	nodes[0] = node0;
	segments[0] = segment0;
	std::unique_ptr<NewNetObject> net_ptr = std::make_unique<NewNetObject>(nodes, segments, scene);
	NewNetObject* ptr = net_ptr.get();
	scene->SetCurrentHover(ptr);
	scene->AddObject(std::move(net_ptr), 0);
	return ptr;
}

node::NewNetObject::NewNetObject(NetNode* startNode, NetNode* endNode, GraphicsScene* scene)
	:GraphicsObject({0,0,0,0}, ObjectType::logic, scene), p_startNode(startNode), p_endNode(endNode)
{
	for (int i = 0; i < 2; i++)
	{
		std::unique_ptr<NetNode> node = std::make_unique<NetNode>(SDL_Point{0,0},scene);
		m_intermediateNodes.push_back(node.get());
		scene->AddObject(std::move(node), NET_NODE_OBJECT_Z);
	}
	for (int i = 0; i < 3; i++)
	{
		NetNode* node1 = nullptr;
		NetNode* node2 = nullptr;
		switch (i)
		{
		case 0:
			node1 = p_startNode;
			node2 = m_intermediateNodes[0];
			break;
		case 1:
			node1 = m_intermediateNodes[0];
			node2 = m_intermediateNodes[1];
			break;
		case 2:
			node1 = m_intermediateNodes[1];
			node2 = p_endNode;
			break;
		}

		std::unique_ptr<NetSegment> segment = std::make_unique<NetSegment>(static_cast<NetOrientation>(i%2),
			node1, node2, scene);
		m_segments.push_back(segment.get());
		scene->AddObject(std::move(segment), NET_SEGMENT_OBJECT_Z);
	}
	UpdateConnectedSegments();
}

node::NewNetObject::NewNetObject(std::array<NetNode*, 4> nodes, std::array<NetSegment*, 3> segments, GraphicsScene* scene)
	: GraphicsObject({ 0,0,0,0 }, ObjectType::logic, scene), p_startNode(nodes[0]), p_endNode(nodes[3]),
	m_intermediateNodes{nodes[1], nodes[2]}, m_segments{segments.begin(), segments.end()}
{
	UpdateConnectedSegments();
}

void node::NewNetObject::OnMouseMove(const SDL_Point& current_mouse_point)
{
	auto* socket_ptr = GetScene()->GetSocketAt(current_mouse_point);
	if (socket_ptr && (socket_ptr->GetConnectedNode() == p_endNode || 
		nullptr == socket_ptr->GetConnectedNode()))
	{
		if (!socket_ptr->GetConnectedNode())
		{
			p_endNode->SetConnectedSocket(socket_ptr);
			p_endNode->setCenter(socket_ptr->GetCenter());
		}
	}
	else
	{
		p_endNode->SetConnectedSocket(nullptr);
		p_endNode->setCenter(current_mouse_point);
	}
	p_endNode->UpdateConnectedSegments();
	UpdateConnectedSegments();
}

MI::ClickEvent node::NewNetObject::OnLMBUp(const SDL_Point& current_mouse_point)
{
	UNUSED_PARAM(current_mouse_point);
	GetScene()->PopObject(this);
	return MI::ClickEvent::CAPTURE_END;
}

void node::NewNetObject::Draw(SDL_Renderer* renderer)
{
	UNUSED_PARAM(renderer);
}

void node::NewNetObject::UpdateConnectedSegments()
{
	if (p_startNode->getCenter().y == p_endNode->getCenter().y)
	{
		UpdateToZmode();
	}
	else if (p_startNode->getCenter().x == p_endNode->getCenter().x)
	{
		UpdateToZmode();
	}
	else
	{
		UpdateToZmode();
	}
}

void node::NewNetObject::UpdateToHorizontal()
{
	SDL_Point center_point = { 
		(p_startNode->getCenter().x + p_endNode->getCenter().x) / 2,
		(p_startNode->getCenter().y + p_endNode->getCenter().y) / 2
	};
	m_intermediateNodes[0]->setCenter(center_point);
	m_intermediateNodes[1]->setCenter(center_point);
	m_segments[0]->Disconnect();
	m_segments[0]->Connect(p_startNode, m_intermediateNodes[0], NetOrientation::Horizontal);
	m_segments[1]->Disconnect();
	m_segments[1]->Connect(m_intermediateNodes[0], m_intermediateNodes[1], NetOrientation::Vertical);
	m_segments[2]->Disconnect();
	m_segments[2]->Connect(m_intermediateNodes[1], p_endNode, NetOrientation::Horizontal);
}

void node::NewNetObject::UpdateToVertical()
{
	SDL_Point center_point = {
		(p_startNode->getCenter().x + p_endNode->getCenter().x) / 2,
		(p_startNode->getCenter().y + p_endNode->getCenter().y) / 2
	};
	m_intermediateNodes[0]->setCenter(center_point);
	m_intermediateNodes[1]->setCenter(center_point);
	m_segments[0]->Disconnect();
	m_segments[0]->Connect(p_startNode, m_intermediateNodes[0], NetOrientation::Vertical);
	m_segments[1]->Disconnect();
	m_segments[1]->Connect(m_intermediateNodes[0], m_intermediateNodes[1], NetOrientation::Horizontal);
	m_segments[2]->Disconnect();
	m_segments[2]->Connect(m_intermediateNodes[1], p_endNode, NetOrientation::Vertical);
}

void node::NewNetObject::UpdateToZmode()
{
	SDL_Point center_point = {
		(p_startNode->getCenter().x + p_endNode->getCenter().x) / 2,
		(p_startNode->getCenter().y + p_endNode->getCenter().y) / 2
	};
	m_intermediateNodes[0]->setCenter({ center_point.x, p_startNode->getCenter().y });
	m_intermediateNodes[1]->setCenter({ center_point.x, p_endNode->getCenter().y });
	m_segments[0]->Disconnect();
	m_segments[0]->Connect(p_startNode, m_intermediateNodes[0], NetOrientation::Horizontal);
	m_segments[1]->Disconnect();
	m_segments[1]->Connect(m_intermediateNodes[0], m_intermediateNodes[1], NetOrientation::Vertical);
	m_segments[2]->Disconnect();
	m_segments[2]->Connect(m_intermediateNodes[1], p_endNode, NetOrientation::Horizontal);

}


