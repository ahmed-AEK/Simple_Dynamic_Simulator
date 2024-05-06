#include "DraggableObject.hpp"

#include "GraphicsScene.hpp"

node::DraggableObject::DraggableObject(const SDL_Rect& sceneRect, ObjectType type, node::GraphicsScene* parent)
: node::GraphicsObject(sceneRect, type, parent)
{
    b_draggable = true;
}

void node::DraggableObject::OnMouseMove(const SDL_Point& current_mouse_point)
{
    return node::GraphicsObject::OnMouseMove(current_mouse_point);
}

MI::ClickEvent node::DraggableObject::OnLMBDown(const SDL_Point& current_mouse_point)
{
    return GraphicsObject::OnLMBDown(current_mouse_point);
}

MI::ClickEvent node::DraggableObject::OnLMBUp(const SDL_Point& current_mouse_point)
{
    return node::GraphicsObject::OnLMBUp(current_mouse_point);
}