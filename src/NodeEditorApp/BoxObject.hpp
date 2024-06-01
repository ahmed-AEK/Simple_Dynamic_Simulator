#pragma once

#include "GraphicsScene/DraggableObject.hpp"

namespace node{

class BoxObject: public node::DraggableObject
{
public:
    BoxObject(SDL_Rect sceneRect, GraphicsScene* scene);
    virtual void Draw(SDL_Renderer* renderer) override;
    virtual MI::ClickEvent OnLMBDown(const SDL_Point& current_mouse_point) override;
    virtual MI::ClickEvent OnRMBUp(const SDL_Point& current_mouse_point) override;
};
}