#include "BlockObject.hpp"
#include "SDL_Framework/SDL_headers.h"
#include "BlockSocketObject.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "IGraphicsScene.hpp"
#include "IGraphicsSceneController.hpp"
#include <algorithm>
#include <iterator>
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include <cassert>

node::BlockObject::BlockObject(model::Rect rect, IGraphicsScene* scene)
	: DraggableObject(rect, ObjectType::node, scene)
{
}

void node::BlockObject::Draw(SDL_Renderer* renderer)
{
    assert(GetScene());

    if (!GetScene()->IsObjectSelected(*this))
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 235, 128, 52, 255);
    }

    SDL_Rect screenRect = GetScene()->GetSpaceScreenTransformer().SpaceToScreenRect(GetSpaceRect());
	SDL_RenderFillRect(renderer, &screenRect);
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
	SDL_Rect inner_rect{ screenRect.x + 2, screenRect.y + 2, screenRect.w - 4, screenRect.h - 4 };
	SDL_RenderFillRect(renderer, &inner_rect);

    for (auto&& sock : m_input_sockets)
    {
        sock.socket->Draw(renderer);
    }
    for (auto&& sock : m_output_sockets)
    {
        sock.socket->Draw(renderer);
    }

}

MI::ClickEvent node::BlockObject::OnLMBDown(const model::Point& current_mouse_point)
{
    auto&& scene = GetScene();
    if (!scene)
    {
        return MI::ClickEvent::NONE;
    }
    auto&& controller = scene->GetController();
    if (!controller)
    {
        return MI::ClickEvent::NONE;
    }
    assert(GetScene());
    auto&& transformer = GetScene()->GetSpaceScreenTransformer();
    SDL_Point space_point = transformer.SpaceToScreenPoint(current_mouse_point);
    return controller->OnBlockLMBDown(space_point, *this);
}

void node::BlockObject::AddInputSocket(int id)
{
    std::unique_ptr<node::BlockSocketObject> sock = std::make_unique<node::BlockSocketObject>(
        node::SocketType::input, GetScene(), this);
    m_input_sockets.push_back({ id,std::move(sock) });
    PositionSockets();

}

void node::BlockObject::AddOutputSocket(int id)
{
    std::unique_ptr<node::BlockSocketObject> sock = std::make_unique<node::BlockSocketObject>(
        node::SocketType::output, GetScene(), this);
    m_output_sockets.push_back({ id,std::move(sock) });
    PositionSockets();

}

std::vector<node::BlockSocketObject*> node::BlockObject::GetSockets()
{
    std::vector<node::BlockSocketObject*> out;
    out.reserve(m_input_sockets.size() + m_output_sockets.size());

    std::transform(m_input_sockets.begin(), m_input_sockets.end(), std::back_inserter(out), [](const auto& item) { return item.socket.get(); });
    std::transform(m_output_sockets.begin(), m_output_sockets.end(), std::back_inserter(out), [](const auto& item) { return item.socket.get(); });
    
    return out;
}

void node::BlockObject::DisconnectSockets()
{
    for (auto& socketData : m_input_sockets)
    {
        if (socketData.socket->GetConnectedNode())
        {
            socketData.socket->SetConnectedNode(nullptr);
        }
    }

    for (auto& socketData : m_output_sockets)
    {
        if (socketData.socket->GetConnectedNode())
        {
            socketData.socket->SetConnectedNode(nullptr);
        }
    }
}

void node::BlockObject::OnSetSpaceRect(const model::Rect& rect)
{
    DraggableObject::OnSetSpaceRect(rect);
    PositionSockets();
}

void node::BlockObject::PositionSockets()
{
    int in_spacing = static_cast<int>(GetSpaceRect().h / (m_input_sockets.size() + 1));
    int in_counter = 1;
    for (auto&& sock : m_input_sockets)
    {
        sock.socket->SetPosition({ GetSpaceRect().x + 2, GetSpaceRect().y + in_spacing * in_counter - node::BlockSocketObject::nodeLength/2});
        in_counter++;
    }
    int out_spacing = static_cast<int>(GetSpaceRect().h / (m_output_sockets.size() + 1));
    int out_counter = 1;
    for (auto&& sock : m_output_sockets)
    {
        sock.socket->SetPosition({ GetSpaceRect().x + GetSpaceRect().w - 2 - sock.socket->nodeLength, GetSpaceRect().y + out_spacing * out_counter - node::BlockSocketObject::nodeLength / 2 });
        out_counter++;
    }
}

node::GraphicsObject* node::BlockObject::OnGetInteractableAtPoint(const model::Point& point)
{
    node::GraphicsObject* hover = nullptr;
    for (auto&& sock : m_input_sockets)
    {
        hover = sock.socket->GetInteractableAtPoint(point);
        if (hover)
        {
            return hover;
        }
    }
    for (auto&& sock : m_output_sockets)
    {
        hover = sock.socket->GetInteractableAtPoint(point);
        if (hover)
        {
            return hover;
        }
    }
    return this;
}

void node::BlockObject::OnUpdateRect()
{
    DraggableObject::OnUpdateRect();
    for (auto&& sock : m_input_sockets)
    {
        sock.socket->UpdateRect();
    }
    for (auto&& sock : m_output_sockets)
    {
        sock.socket->UpdateRect();
    }
}

