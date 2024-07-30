#include "BlockDragLogic.hpp"
#include "BlockObject.hpp"
#include "GraphicsScene.hpp"

node::logic::BlockDragLogic::BlockDragLogic(model::Point startPointMouseSpace, model::Point startObjectEdge, BlockObject& block, GraphicsScene* scene)
    :GraphicsLogic(scene), m_startPointMouseSpace{ startPointMouseSpace }, m_startObjectEdge{ startObjectEdge }, m_block{ block.GetMIHandlePtr()}
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


