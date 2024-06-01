#include "BoxObject.hpp"
#include "ExampleContextMenu.hpp"
#include "GraphicsScene/GraphicsScene.hpp"
#include "toolgui/Scene.hpp"

node::BoxObject::BoxObject(SDL_Rect sceneRect, GraphicsScene* scene)
: node::DraggableObject(sceneRect, ObjectType::logic, scene)
{

}

void node::BoxObject::Draw(SDL_Renderer* renderer)
{
    if (!GetScene()->isObjectSelected(*this))
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    }
    SDL_RenderFillRect(renderer, &GetRect());
}


MI::ClickEvent node::BoxObject::OnLMBDown(const SDL_Point& p)
{
    SDL_Log("Box Pressed!");
    auto&& ret = DraggableObject::OnLMBDown(p);
    return ret;
}

MI::ClickEvent node::BoxObject::OnRMBUp(const SDL_Point& p)
{
    SDL_Log("Box RMB!");
    std::unique_ptr<node::ContextMenu> menu = std::make_unique<node::ExampleContextMenu>(this->GetScene()->GetScene());
    this->GetScene()->GetScene()->ShowContextMenu(std::move(menu), {p.x, p.y});
    return MI::ClickEvent::CLICKED;
}
