#include "BlockDeleteLogic.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"
#include "NetUtils/DeleteHelpers.hpp"

node::logic::BlockDeleteLogic::BlockDeleteLogic(BlockObject& block, GraphicsScene* scene, GraphicsObjectsManager* manager)
	:GraphicsLogic{scene, manager}, m_block{block}
{
}

void node::logic::BlockDeleteLogic::OnMouseMove(const model::Point& current_mouse_point)
{
	const auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_block.isAlive() || new_hover != m_block.GetObjectPtr())
	{
		GetScene()->SetGraphicsLogic(nullptr);
	}
}

MI::ClickEvent node::logic::BlockDeleteLogic::OnLMBUp(const model::Point& current_mouse_point)
{
	const auto* new_hover = GetScene()->GetObjectAt(current_mouse_point);
	if (!m_block.isAlive() || new_hover != m_block.GetObjectPtr())
	{
		return MI::ClickEvent::CAPTURE_END;
	}
	auto block_id = m_block->GetModelId();
	if (!block_id)
	{
		return MI::ClickEvent::CAPTURE_END;
	}

	auto request = NetUtils::GetDeletionRequestForBlock(*block_id, GetObjectsManager()->GetSceneModel()->GetModel());
	if (request)
	{
		GetObjectsManager()->GetSceneModel()->RemoveBlock(std::move(*request));
	}
	return MI::ClickEvent::CAPTURE_END;
}


