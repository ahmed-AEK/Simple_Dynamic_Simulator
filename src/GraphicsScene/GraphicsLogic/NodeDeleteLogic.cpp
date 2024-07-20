#include "GraphicsLogic/NodeDeleteLogic.hpp"

#include "BlockObject.hpp"
#include "GraphicsScene.hpp"

node::NodeDeleteLogic::NodeDeleteLogic(BlockObject& node, GraphicsScene* scene)
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
	SDL_Rect objectSpaceRect = ToSDLRect(object_ptr->GetSpaceRect());
	if (!SDL_PointInRect(&current_mouse_point, &objectSpaceRect))
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return;
	}
}

MI::ClickEvent node::NodeDeleteLogic::OnLMBUp(const SDL_Point& current_mouse_point)
{
	if (!m_object.isAlive())
	{
		return MI::ClickEvent::NONE;
	}

	SDL_Rect ObjectSpaceRect = ToSDLRect(m_object.GetObjectPtr()->GetSpaceRect());
	if(SDL_PointInRect(&current_mouse_point, &ObjectSpaceRect) &&
		(GraphicsSceneMode::Delete == GetScene()->GetMode()))
	{
		auto node_ptr = static_cast<BlockObject*>(m_object.GetObjectPtr());
		GetScene()->SetCurrentHover(nullptr);
		node_ptr->DisconnectSockets();
		auto ptr = GetScene()->PopObject(node_ptr);
		SetDone();
		return MI::ClickEvent::CLICKED;
	}

	return MI::ClickEvent::NONE;
}
