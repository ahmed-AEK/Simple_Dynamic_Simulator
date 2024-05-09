#include "Node.hpp"
#include "SDL_Framework/SDL_headers.h"
#include "GraphicsScene.hpp"
#include "NodeSocket.hpp"
#include "toolgui/NetObject.hpp"

node::Node::Node(SDL_Rect rect, GraphicsScene* scene)
	: DraggableObject(rect, ObjectType::node, scene)
{
}

void node::Node::Draw(SDL_Renderer* renderer)
{

    if (!GetScene()->isObjectSelected(*this))
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 235, 128, 52, 255);
    }
	SDL_RenderFillRect(renderer, &GetRect());
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
	SDL_Rect inner_rect{ GetRect().x + 2, GetRect().y + 2, GetRect().w - 4, GetRect().h - 4 };
	SDL_RenderFillRect(renderer, &inner_rect);

    for (auto&& sock : m_input_sockets)
    {
        sock.socket->Draw(renderer);
    }
    for (auto&& sock : m_output_sockets)
    {
        sock.socket->Draw(renderer);
    }
    for (auto&& sock : m_inout_sockets)
    {
        sock.socket->Draw(renderer);
    }

}

MI::ClickEvent node::Node::OnLMBDown(const SDL_Point& current_mouse_point)
{
    if (GetScene()->GetMode() == GraphicsSceneMode::Delete)
    {
        b_being_deleted = true;
        return MI::ClickEvent::CAPTURE_START;
    }
    return DraggableObject::OnLMBUp(current_mouse_point);
}

MI::ClickEvent node::Node::OnLMBUp(const SDL_Point& current_mouse_point)
{
    if (b_being_deleted && GraphicsSceneMode::Delete == GetScene()->GetMode())
    {
        GetScene()->SetCurrentHover(nullptr);
        DisconnectSockets();
        auto ptr = GetScene()->PopObject(this);
        return MI::ClickEvent::CAPTURE_END;
    }
    return DraggableObject::OnLMBUp(current_mouse_point);
}

void node::Node::OnMouseMove(const SDL_Point& current_mouse_point)
{
    if (b_being_deleted && !SDL_PointInRect(&current_mouse_point, &GetSpaceRect()))
    {
        b_being_deleted = false;
    }
}


void node::Node::AddInputSocket(int id)
{
    std::unique_ptr<node::NodeSocket> sock = std::make_unique<node::NodeSocket>(
        node::SocketType::input, GetScene(), this);
    m_input_sockets.push_back({ id,std::move(sock) });
    PositionSockets();

}

void node::Node::AddOutputSocket(int id)
{
    std::unique_ptr<node::NodeSocket> sock = std::make_unique<node::NodeSocket>(
        node::SocketType::output, GetScene(), this);
    m_output_sockets.push_back({ id,std::move(sock) });
    PositionSockets();

}

std::vector<node::NodeSocket*> node::Node::GetSockets()
{
    std::vector<node::NodeSocket*> out;
    out.reserve(m_input_sockets.size() + m_output_sockets.size() + m_inout_sockets.size());
    for (auto& item : m_input_sockets)
    {
        out.push_back(item.socket.get());
    }
    for (auto& item : m_output_sockets)
    {
        out.push_back(item.socket.get());
    }
    for (auto& item : m_inout_sockets)
    {
        out.push_back(item.socket.get());
    }
    return out;
}

void node::Node::DisconnectSockets()
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

    for (auto& socketData : m_inout_sockets)
    {
        if (socketData.socket->GetConnectedNode())
        {
            socketData.socket->SetConnectedNode(nullptr);
        }
    }
}

void node::Node::OnSetSpaceRect(const SDL_Rect& rect)
{
    DraggableObject::OnSetSpaceRect(rect);
    PositionSockets();
}

void node::Node::PositionSockets()
{
    int in_spacing = static_cast<int>(GetSpaceRect().h / (m_input_sockets.size() + 1));
    int in_counter = 1;
    for (auto&& sock : m_input_sockets)
    {
        sock.socket->SetPosition({ GetSpaceRect().x + 2, GetSpaceRect().y + in_spacing * in_counter - node::NodeSocket::nodeLength/2});
        in_counter++;
    }
    int out_spacing = static_cast<int>(GetSpaceRect().h / (m_output_sockets.size() + 1));
    int out_counter = 1;
    for (auto&& sock : m_output_sockets)
    {
        sock.socket->SetPosition({ GetSpaceRect().x + GetSpaceRect().w - 2 - sock.socket->nodeLength, GetSpaceRect().y + out_spacing * out_counter - node::NodeSocket::nodeLength / 2 });
        out_counter++;
    }
}

node::GraphicsObject* node::Node::OnGetInteractableAtPoint(const SDL_Point& point)
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
    for (auto&& sock : m_inout_sockets)
    {
        hover = sock.socket->GetInteractableAtPoint(point);
        if (hover)
        {
            return hover;
        }
    }
    return this;
}

void node::Node::OnUpdateRect()
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
    for (auto&& sock : m_inout_sockets)
    {
        sock.socket->UpdateRect();
    }
}

