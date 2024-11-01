#include "GraphicsLogic/NodeDeleteLogic.hpp"

#include "BlockObject.hpp"
#include "GraphicsScene.hpp"

node::NodeDeleteLogic::NodeDeleteLogic(BlockObject& node, GraphicsScene* scene, GraphicsObjectsManager* manager)
	: GraphicsLogic{scene, manager }, m_object{node.GetFocusHandlePtr()}
{
}

void node::NodeDeleteLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	if (!m_object.isAlive())
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return;
	}

	auto object_ptr = m_object.GetObjectPtr();
	SDL_FRect objectSpaceRect = ToSDLRect(object_ptr->GetSpaceRect());
	SDL_FPoint current_mouse_sdl = ToSDLPoint(current_mouse_point);
	if (!SDL_PointInRectFloat(&current_mouse_sdl, &objectSpaceRect))
	{
		GetScene()->SetGraphicsLogic(nullptr);
		return;
	}
}

MI::ClickEvent node::NodeDeleteLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	
	UNUSED_PARAM(current_mouse_point);

	return MI::ClickEvent::NONE;
}
