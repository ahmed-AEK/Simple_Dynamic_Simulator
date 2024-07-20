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
#include "NodeModels/BlockModel.hpp"
#include "NodeSDLStylers/BlockStyler.hpp"

node::BlockObject::BlockObject(IGraphicsScene* scene, std::shared_ptr<model::BlockModel> model, std::shared_ptr<BlockStyler> styler)
    : DraggableObject((model ? model->GetBounds() : model::Rect{0,0,0,0}), ObjectType::node, scene), m_model{std::move(model)}, m_styler{std::move(styler)}
{
    for (const auto& in_socket : m_model->GetSockets(model::BlockSocketModel::SocketType::input))
    {
        AddInputSocket(in_socket.GetId());
    }
    for (const auto& out_socket : m_model->GetSockets(model::BlockSocketModel::SocketType::output))
    {
        AddOutputSocket(out_socket.GetId());
    }
}

node::BlockObject::~BlockObject()
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
    

    auto&& transformer = GetScene()->GetSpaceScreenTransformer();
    assert(m_styler);
    m_styler->DrawBlock(renderer, *m_model, transformer, GetScene()->IsObjectSelected(*this));
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

void node::BlockObject::AddInputSocket(model::BlockSocketModel::SocketId id)
{
    auto sock = std::make_unique<node::BlockSocketObject>( id,
        model::BlockSocketModel::SocketType::input, GetScene(), this );
    m_input_sockets.push_back({ std::move(sock) });
}

void node::BlockObject::AddOutputSocket(model::BlockSocketModel::SocketId id)
{
    auto sock = std::make_unique<node::BlockSocketObject>(id,
        model::BlockSocketModel::SocketType::output, GetScene(), this);
    m_output_sockets.push_back({ std::move(sock) });
}

std::vector<node::BlockSocketObject*> node::BlockObject::GetSockets()
{
    std::vector<node::BlockSocketObject*> out;
    out.reserve(m_input_sockets.size() + m_output_sockets.size());

    std::transform(m_input_sockets.begin(), m_input_sockets.end(), std::back_inserter(out), [](const auto& item) { return item.get(); });
    std::transform(m_output_sockets.begin(), m_output_sockets.end(), std::back_inserter(out), [](const auto& item) { return item.get(); });
    
    return out;
}

void node::BlockObject::OnSetSpaceRect(const model::Rect& rect)
{
    if (m_model)
    {
        m_model->SetBounds(rect);
    }
    DraggableObject::OnSetSpaceRect(rect);
    RePositionSockets();
}

void node::BlockObject::RePositionSockets()
{
    if (!m_model)
    {
        return;
    }

    auto&& origin = GetSpaceRect();
    auto&& input_sockets = m_model->GetSockets(model::BlockSocketModel::SocketType::input);
    assert(m_input_sockets.size() == m_model->GetSockets(model::BlockSocketModel::SocketType::input).size());
    for (size_t i = 0; i < m_input_sockets.size(); ++i)
    {
        auto&& position = input_sockets[i].GetPosition();
        m_input_sockets[i]->SetPosition({ origin.x + position.x, origin.y + position.y });
    }

    auto&& output_sockets = m_model->GetSockets(model::BlockSocketModel::SocketType::output);
    assert(m_output_sockets.size() == m_model->GetSockets(model::BlockSocketModel::SocketType::input).size());
    for (size_t i = 0; i < m_input_sockets.size(); ++i)
    {
        auto&& position = output_sockets[i].GetPosition();
        m_output_sockets[i]->SetPosition({ origin.x + position.x, origin.y + position.y });
    }
}

node::GraphicsObject* node::BlockObject::OnGetInteractableAtPoint(const model::Point& point)
{
    node::GraphicsObject* hover = nullptr;
    for (auto&& sock : m_input_sockets)
    {
        hover = sock->GetInteractableAtPoint(point);
        if (hover)
        {
            return hover;
        }
    }
    for (auto&& sock : m_output_sockets)
    {
        hover = sock->GetInteractableAtPoint(point);
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
    RePositionSockets();
}

