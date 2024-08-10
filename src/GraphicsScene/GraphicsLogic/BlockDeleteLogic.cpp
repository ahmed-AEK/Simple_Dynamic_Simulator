#include "BlockDeleteLogic.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"

node::logic::BlockDeleteLogic::BlockDeleteLogic(BlockObject& block, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_block{block.GetFocusHandlePtr()}
{
}

void node::logic::BlockDeleteLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_block.isAlive() || new_hover != m_block.GetObjectPtr())
	{
		GetScene()->SetGraphicsLogic(nullptr);
	}
}

MI::ClickEvent node::logic::BlockDeleteLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_block.isAlive() || new_hover != m_block.GetObjectPtr())
	{
		return MI::ClickEvent::NONE;
	}

	GetObjectsManager()->GetSceneModel()->RemoveBlockById(*static_cast<BlockObject*>(m_block.GetObjectPtr())->GetModelId());

	return MI::ClickEvent::CLICKED;
}

