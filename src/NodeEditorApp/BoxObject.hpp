#pragma once

#include "GraphicsScene/DraggableObject.hpp"

namespace node{

class BoxObject: public node::DraggableObject
{
public:
    BoxObject(model::Rect sceneRect, IGraphicsScene* scene);
    virtual void Draw(SDL_Renderer* renderer) override;
    virtual MI::ClickEvent OnLMBDown(const model::Point& current_mouse_point) override;
    virtual MI::ClickEvent OnRMBUp(const model::Point& current_mouse_point) override;
};
}