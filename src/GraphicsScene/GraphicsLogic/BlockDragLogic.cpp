#include "BlockDragLogic.hpp"
#include "BlockObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"

node::logic::BlockDragLogic::BlockDragLogic(model::Point startPointMouseSpace, 
    model::Point startObjectEdge, BlockObject& block, GraphicsScene* scene, GraphicsObjectsManager* manager)
    :GraphicsLogic(scene, manager), m_startPointMouseSpace{ startPointMouseSpace }, 
    m_startObjectEdge{ startObjectEdge }, m_block{ block.GetMIHandlePtr()}
{
}

void node::logic::BlockDragLogic::OnMouseMove(const model::Point& current_mouse_point)
{
    auto drag_object = m_block.GetObjectPtr();
    if (!drag_object)
    {
        GetScene()->SetGraphicsLogic(nullptr);
        return;
    }


    const auto drag_vector = model::Point{ 
        current_mouse_point.x - m_startPointMouseSpace.x, 
        current_mouse_point.y - m_startPointMouseSpace.y };


    drag_object->SetSpaceOrigin(GetScene()->QuantizePoint({
        m_startObjectEdge.x + drag_vector.x,
        m_startObjectEdge.y + drag_vector.y
        }));
    
    GetScene()->UpdateObjectsRect();
    GetScene()->InvalidateRect();
}

void node::logic::BlockDragLogic::OnCancel()
{
    auto drag_object = m_block.GetObjectPtr();
    if (!drag_object)
    {
        return;
    }
    drag_object->SetSpaceOrigin(GetScene()->QuantizePoint({
        m_startObjectEdge.x,
        m_startObjectEdge.y
        }));

}

MI::ClickEvent node::logic::BlockDragLogic::OnLMBUp(const model::Point& current_mouse_point)
{
    auto* scene = GetScene();
    auto* block = static_cast<BlockObject*>(m_block.GetObjectPtr());
    if (!scene && !block)
    {
        return MI::ClickEvent::NONE;
    }

    const auto drag_vector = model::Point{
    current_mouse_point.x - m_startPointMouseSpace.x,
    current_mouse_point.y - m_startPointMouseSpace.y };

    assert(GetObjectsManager());
    auto block_id = block->GetModelId();
    assert(block_id);
    GetObjectsManager()->GetSceneModel()->MoveBlockById(*block_id, scene->QuantizePoint({
        m_startObjectEdge.x + drag_vector.x,
        m_startObjectEdge.y + drag_vector.y
        }));

    return MI::ClickEvent::CLICKED;
}

