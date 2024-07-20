#include "DraggableObject.hpp"

node::DraggableObject::DraggableObject(const model::Rect& sceneRect, ObjectType type, node::IGraphicsScene* parent)
: node::GraphicsObject(sceneRect, type, parent)
{
    b_draggable = true;
}

void node::DraggableObject::OnMouseMove(const model::Point& current_mouse_point)
{
    return node::GraphicsObject::OnMouseMove(current_mouse_point);
}

MI::ClickEvent node::DraggableObject::OnLMBDown(const model::Point& current_mouse_point)
{
    return GraphicsObject::OnLMBDown(current_mouse_point);
}

MI::ClickEvent node::DraggableObject::OnLMBUp(const model::Point& current_mouse_point)
{
    return node::GraphicsObject::OnLMBUp(current_mouse_point);
}