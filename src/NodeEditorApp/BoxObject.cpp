#include "BoxObject.hpp"
#include "ExampleContextMenu.hpp"
#include "GraphicsScene/IGraphicsScene.hpp"
#include "toolgui/Scene.hpp"
#include <cassert>
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"

node::BoxObject::BoxObject(model::Rect sceneRect, IGraphicsScene* scene)
: node::DraggableObject(sceneRect, ObjectType::interactive, scene)
{

}

void node::BoxObject::Draw(SDL_Renderer* renderer)
{
    if (!GetScene()->IsObjectSelected(*this))
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    }
    assert(GetScene());
    SDL_Rect screenRect = GetScene()->GetSpaceScreenTransformer().SpaceToScreenRect(GetSpaceRect());
    SDL_RenderFillRect(renderer, &screenRect);
}


MI::ClickEvent node::BoxObject::OnLMBDown(const model::Point& p)
{
    SDL_Log("Box Pressed!");
    auto&& ret = DraggableObject::OnLMBDown(p);
    return ret;
}

MI::ClickEvent node::BoxObject::OnRMBUp(const model::Point& p)
{
    UNUSED_PARAM(p);
    /*
    SDL_Log("Box RMB!");
    std::unique_ptr<node::ContextMenu> menu = std::make_unique<node::ExampleContextMenu>(this->GetScene()->GetScene());
    this->GetScene()->GetScene()->ShowContextMenu(std::move(menu), {p.x, p.y});
    return MI::ClickEvent::CLICKED;
    */
    return MI::ClickEvent::NONE;
}
