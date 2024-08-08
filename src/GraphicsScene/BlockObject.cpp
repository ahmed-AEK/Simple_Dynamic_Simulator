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

std::unique_ptr<node::BlockObject> node::BlockObject::Create(IGraphicsScene* scene, const model::BlockModel& model, std::shared_ptr<BlockStyler> styler)
{
    auto ptr = std::make_unique<BlockObject>(scene, model.GetBounds(), std::move(styler), model.GetId());
    for (const auto& socket : model.GetSockets())
    {
        auto socket_ptr = std::make_unique<BlockSocketObject>(socket.GetType(), socket.GetId(), 
            socket.GetPosition(), scene, ptr.get());
        ptr->AddSocket(std::move(socket_ptr));
    }
    return ptr;
}

node::BlockObject::BlockObject(IGraphicsScene* scene, const model::Rect& rect,
    std::shared_ptr<BlockStyler> styler, std::optional<model::BlockId> model_id)
    :GraphicsObject{rect, ObjectType::block, scene}, m_id{model_id}, m_styler{std::move(styler)}
{
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
        m_styler->DrawBlockSocket(renderer, socket->GetCenterInSpace(), transformer, socket->GetSocketType());
    }
}

MI::ClickEvent node::BlockObject::OnLMBDown(const model::Point& current_mouse_point)
{
    return GraphicsObject::OnLMBDown(current_mouse_point);
}

void node::BlockObject::AddSocket(std::unique_ptr<BlockSocketObject> socket)
{
    auto&& rect = GetSpaceRect();
    auto&& origin = model::Util::get_rect_origin(rect);
    socket->SetCenterInSpace(socket->GetCenterInBlock() + origin);
    m_sockets.push_back(std::move(socket));
}

std::optional<node::model::BlockId> node::BlockObject::GetModelId()
{
    return m_id;
}

const std::vector<std::unique_ptr<node::BlockSocketObject>>& node::BlockObject::GetSockets() const
{
    return m_sockets;
}

void node::BlockObject::OnSetSpaceRect(const model::Rect& rect)
{
    GraphicsObject::OnSetSpaceRect(rect);
    RePositionSockets();
}

void node::BlockObject::RePositionSockets()
{
    auto&& block_rect = GetSpaceRect();
    auto&& origin = model::Util::get_rect_origin(block_rect);
    for (auto&& socket : m_sockets)
    {
        auto& sock_pos = socket->GetCenterInBlock();
        socket->SetCenterInSpace(sock_pos + origin);
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

