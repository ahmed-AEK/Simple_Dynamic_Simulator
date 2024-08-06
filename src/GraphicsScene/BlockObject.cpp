#include "BlockObject.hpp"
#include "SDL_Framework/SDL_headers.h"
#include "BlockSocketObject.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "IGraphicsScene.hpp"
#include <algorithm>
#include <iterator>
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include <cassert>
#include "NodeModels/BlockModel.hpp"
#include "NodeSDLStylers/BlockStyler.hpp"

node::BlockObject::BlockObject(IGraphicsScene* scene, std::shared_ptr<model::BlockModel> model, std::shared_ptr<BlockStyler> styler)
    : GraphicsObject((model ? model->GetBounds() : model::Rect{100,100,100,100}), ObjectType::block, scene), m_model{std::move(model)}, m_styler{std::move(styler)}
{
    if (!m_model)
    {
        return;
    }

    for (const auto& socket : m_model->GetSockets())
    {
        switch (socket.GetType())
        {
        case model::BlockSocketModel::SocketType::input:
        {
            AddInputSocket(socket.GetId());
            break;
        }
        case model::BlockSocketModel::SocketType::output:
        {
            AddOutputSocket(socket.GetId());
            break;
        }
        }
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
    m_styler->DrawBlockOutline(renderer, GetSpaceRect(), transformer, GetScene()->IsObjectSelected(*this));
    for (const auto& socket : m_sockets)
    {
        m_styler->DrawBlockSocket(renderer, socket->GetCenter(), transformer, socket->GetSocketType());
    }
}

MI::ClickEvent node::BlockObject::OnLMBDown(const model::Point& current_mouse_point)
{
    return GraphicsObject::OnLMBDown(current_mouse_point);
}

void node::BlockObject::AddInputSocket(model::SocketId id)
{
    auto sock = std::make_unique<node::BlockSocketObject>( id,
        model::BlockSocketModel::SocketType::input, GetScene(), this );
    m_sockets.push_back({ std::move(sock) });
}

void node::BlockObject::AddOutputSocket(model::SocketId id)
{
    auto sock = std::make_unique<node::BlockSocketObject>(id,
        model::BlockSocketModel::SocketType::output, GetScene(), this);
    m_sockets.push_back({ std::move(sock) });
}

node::model::BlockId node::BlockObject::GetModelId()
{
    return m_model->GetId();
}

std::vector<node::BlockSocketObject*> node::BlockObject::GetSockets()
{
    std::vector<node::BlockSocketObject*> out;
    out.reserve(m_sockets.size());

    std::transform(m_sockets.begin(), m_sockets.end(), std::back_inserter(out), [](const auto& item) { return item.get(); });
    return out;
}

void node::BlockObject::OnSetSpaceRect(const model::Rect& rect)
{
    GraphicsObject::OnSetSpaceRect(rect);
    RePositionSockets();
}

void node::BlockObject::RePositionSockets()
{
    if (!m_model)
    {
        return;
    }

    auto&& origin = GetSpaceRect();
    assert(m_sockets.size() == m_model->GetSockets().size());
    for (auto&& sock: m_sockets)
    {
        auto sock_model = m_model->GetSocketById(sock->GetId());
        assert(sock_model);
        auto&& position = (*sock_model).get().GetPosition();
        sock->SetPosition({ origin.x + position.x - m_styler->SocketLength / 2, origin.y + position.y - m_styler->SocketLength / 2 });
    }
}

node::GraphicsObject* node::BlockObject::OnGetInteractableAtPoint(const model::Point& point)
{
    node::GraphicsObject* hover = nullptr;
    for (auto&& sock : m_sockets)
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
    GraphicsObject::OnUpdateRect();
    RePositionSockets();
}

