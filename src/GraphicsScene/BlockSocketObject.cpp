#include "BlockSocketObject.hpp"
#include "BlockObject.hpp"
#include "SDL_Framework/SDL_headers.h"
#include "NetObject.hpp"
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include "SDL_Framework/SDLRenderer.hpp"
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

std::optional<node::model::SocketUniqueId> node::BlockSocketObject::GetUniqueId()
{
	if (!m_id)
	{
		return std::nullopt;
	}
	if (!m_parentBlock)
	{
		return std::nullopt;
	}
	auto parent_id_opt = m_parentBlock->GetModelId();
	if (!parent_id_opt)
	{
		return std::nullopt;
	}

	return model::SocketUniqueId{*m_id, *parent_id_opt};
}

node::BlockSocketObject::BlockSocketObject(model::BlockSocketModel::SocketType type, std::optional<model::SocketId> id, 
	model::Point center_in_block)
	:GraphicsObject{ model::ObjectSize{ nodeLength,nodeLength },ObjectType::socket, nullptr}, 
	m_socktType(type), m_id{ id }
{
	SetSelectable(false);
	SetCenterInBlock(center_in_block);
}

void node::BlockSocketObject::Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer)
{
	UNUSED_PARAM(renderer);
	UNUSED_PARAM(transformer);
}

void node::BlockSocketObject::UpdateConnectedNodes()
{
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

void node::BlockSocketObject::OnSetPosition(const model::Point& position)
{
	GraphicsObject::OnSetPosition(position);
	UpdateConnectedNodes();
}

