#include "BlockSocketObject.hpp"
#include "BlockObject.hpp"
#include "NetObject.hpp"

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

std::optional<node::model::SocketUniqueId> node::BlockSocketObject::GetUniqueId() const
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
	model::Point center_in_block, model::ConnectedSegmentSide connection_side)
	:GraphicsObject{ model::ObjectSize{ nodeLength,nodeLength },ObjectType::socket, nullptr}, 
	m_socktType(type), m_connection_side{ connection_side }, m_id{ id }
{
	SetSelectable(false);
	SetCenterInBlock(center_in_block);
}

void node::BlockSocketObject::Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer)
{
	UNUSED_PARAM(renderer);
	UNUSED_PARAM(transformer);
}

void node::BlockSocketObject::OnSetPosition(const model::Point& position)
{
	GraphicsObject::OnSetPosition(position);
}

