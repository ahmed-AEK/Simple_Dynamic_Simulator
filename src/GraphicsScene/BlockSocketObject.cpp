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


node::BlockSocketObject::BlockSocketObject(model::BlockSocketModel::SocketType type, std::optional<model::SocketId> id, 
	model::Point center_in_block, IGraphicsScene* parentScene, BlockObject* parentBlock)
	:GraphicsObject{model::Rect{ 0,0,nodeLength,nodeLength },ObjectType::socket, parentScene}, 
	m_center_in_block{ center_in_block }, m_parentBlock(parentBlock), m_socktType(type), m_id{ id }
{
	b_selectable = false;
	b_draggable = false;
}

node::model::Point node::BlockSocketObject::GetCenterInSpace()
{
	return { GetSpaceRect().x + nodeLength / 2, GetSpaceRect().y + nodeLength / 2 };

}

void node::BlockSocketObject::SetCenterInSpace(const model::Point& point)
{
	SetSpaceRect({point.x - nodeLength/2, point.y - nodeLength/2, nodeLength, nodeLength});
}

void node::BlockSocketObject::Draw(SDL_Renderer* renderer)
{
	UNUSED_PARAM(renderer);
}

void node::BlockSocketObject::OnSetSpaceRect(const model::Rect& rect)
{
	GraphicsObject::OnSetSpaceRect(rect);
	if (m_connected_node)
	{
		const auto& new_center = GetCenterInSpace();
		m_connected_node->setCenter(new_center);
		assert(m_connected_node->GetConnectedSegmentsCount() == 1);

		// set connected node position
		NetSegment* next_segment = m_connected_node->getSegment(model::ConnectedSegmentSide::east);
		if (!next_segment)
		{
			next_segment = m_connected_node->getSegment(model::ConnectedSegmentSide::west);
		}

		NetNode* next_node = next_segment->getStartNode();
		if (next_node == m_connected_node)
		{
			next_node = next_segment->getEndNode();
		}

		// set position of the first node after one segment
		next_node->setCenter({ next_node->getCenter().x, new_center.y });
		next_node->UpdateConnectedSegments();
	}
}

