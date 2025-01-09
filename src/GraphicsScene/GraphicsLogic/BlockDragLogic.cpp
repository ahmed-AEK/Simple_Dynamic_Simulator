#include "BlockDragLogic.hpp"
#include "BlockObject.hpp"
#include "GraphicsScene.hpp"
#include "GraphicsObjectsManager.hpp"

node::logic::BlockDragLogic::BlockDragLogic(model::Point startPointMouseSpace, 
    model::Point startObjectEdge, BlockObject& block, GraphicsScene* scene, GraphicsObjectsManager* manager)
    :GraphicsLogic(scene, manager), m_startPointMouseSpace{ startPointMouseSpace }, 
    m_startObjectEdge{ startObjectEdge }, m_block{ block.GetMIHandlePtr()}
{
    assert(scene);
    assert(manager);
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


    drag_object->SetPosition(GetScene()->QuantizePoint({
        m_startObjectEdge.x + drag_vector.x,
        m_startObjectEdge.y + drag_vector.y
        }));
}

void node::logic::BlockDragLogic::OnCancel()
{
    auto drag_object = m_block.GetObjectPtr();
    if (!drag_object)
    {
        return;
    }
    drag_object->SetPosition(GetScene()->QuantizePoint({
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
        return MI::ClickEvent::CAPTURE_END;
    }

    const auto drag_vector = model::Point{
    current_mouse_point.x - m_startPointMouseSpace.x,
    current_mouse_point.y - m_startPointMouseSpace.y };

    // undo motion for now
    block->SetPosition(GetScene()->QuantizePoint({
    m_startObjectEdge.x,
    m_startObjectEdge.y
        }));

    {
        // nothing to do if drag is zero.
        model::Point quantized_drag_vector = scene->QuantizePoint(drag_vector + m_startObjectEdge);
        if (quantized_drag_vector == m_startObjectEdge)
        {
            return MI::ClickEvent::CAPTURE_END;
        }
    }

    assert(GetObjectsManager());
    auto block_id = block->GetModelId();
    assert(block_id);
    GetObjectsManager()->GetSceneModel()->MoveBlockById(*block_id, scene->QuantizePoint(drag_vector + m_startObjectEdge));

    return MI::ClickEvent::CAPTURE_END;
}


