#include "GraphicsLogic/NodeDeleteLogic.hpp"

#include "Node.hpp"
#include "GraphicsScene.hpp"

node::NodeDeleteLogic::NodeDeleteLogic(Node& node, GraphicsScene* scene)
	: GraphicsLogic{scene}, m_object{node.GetFocusHandlePtr()}
{
}

void node::NodeDeleteLogic::OnMouseMove(const SDL_Point& current_mouse_point)
{
	if (!m_object.isAlive())
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return;
	}

	auto object_ptr = m_object.GetObjectPtr();
	if (!SDL_PointInRect(&current_mouse_point, &object_ptr->GetSpaceRect()))
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return;
	}
}

MI::ClickEvent node::NodeDeleteLogic::OnLMBUp(const SDL_Point& current_mouse_point)
{
	if (m_object.isAlive() && 
		SDL_PointInRect(&current_mouse_point, &m_object.GetObjectPtr()->GetSpaceRect()) &&
		(GraphicsSceneMode::Delete == GetScene()->GetMode()))
	{
		auto node_ptr = static_cast<Node*>(m_object.GetObjectPtr());
		GetScene()->SetCurrentHover(nullptr);
		node_ptr->DisconnectSockets();
		auto ptr = GetScene()->PopObject(node_ptr);
		SetDone();
		return MI::ClickEvent::CLICKED;
	}

	return MI::ClickEvent::NONE;
}
