#pragma once

#include "GraphicsScene/GraphicsScene_exports.h"

#include "GraphicsScene/GraphicsObject.hpp"

namespace node
{
class GraphicsScene;

class GRAPHICSSCENE_API DraggableObject: public GraphicsObject
{
public:
    DraggableObject(const SDL_Rect& sceneRect, ObjectType type, node::GraphicsScene* parent);
protected:
    virtual void OnMouseMove(const SDL_Point& current_mouse_point) override;
    virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override;
    virtual MI::ClickEvent OnLMBUp(const SDL_Point& current_mouse_point) override;
    SDL_Point m_dragStartPoint{ 0,0 };
    SDL_Point m_dragStartEdge{ 0,0 };
    bool b_dragging = false;

};
}