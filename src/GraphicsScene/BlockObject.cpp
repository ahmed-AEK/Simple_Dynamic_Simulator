#include "BlockObject.hpp"
#include "SDL_Framework/SDL_headers.h"
#include "BlockSocketObject.hpp"
#include "GraphicsScene/NetObject.hpp"
#include <algorithm>
#include <iterator>
#include "NodeSDLStylers/SpaceScreenTransformer.hpp"
#include <cassert>
#include "NodeModels/BlockModel.hpp"
#include "NodeSDLStylers/BlockStyler.hpp"
#include "GraphicsScene/GraphicsScene.hpp"
#include "GraphicsScene/BlockResizeObject.hpp"

std::unique_ptr<node::BlockObject> node::BlockObject::Create(GraphicsScene* scene, const model::BlockModel& model, std::unique_ptr<BlockStyler> styler)
{
    auto ptr = std::make_unique<BlockObject>(scene, model.GetBounds(), std::move(styler), model.GetId(), model.GetOrienation());
    for (const auto& socket : model.GetSockets())
    {
        auto socket_ptr = std::make_unique<BlockSocketObject>(socket.GetType(), socket.GetId(), 
            socket.GetPosition(), scene, ptr.get());
        ptr->AddSocket(std::move(socket_ptr));
    }
    return ptr;
}

node::BlockObject::BlockObject(GraphicsScene* scene, const model::Rect& rect,
    std::unique_ptr<BlockStyler> styler, std::optional<model::BlockId> model_id, model::BlockOrientation orientation)
    :GraphicsObject{ rect, ObjectType::block, scene }, m_id{ model_id }, m_styler{ std::move(styler) }, m_orientation{ orientation }
{
}

node::BlockObject::~BlockObject()
{
    if (m_resizer)
    {
        m_resizer->SetVisible(false);
    }
}

void node::BlockObject::Draw(SDL_Renderer* renderer, const SpaceScreenTransformer& transformer)
{
    assert(m_styler);
    auto selected = IsSelected();
    m_styler->DrawBlockOutline(renderer, GetSpaceRect(), transformer, GetOrienation(), selected);
    for (const auto& socket : m_sockets)
    {
        m_styler->DrawBlockSocket(renderer, socket->GetCenterInSpace(), transformer, socket->GetSocketType());
    }
    m_styler->DrawBlockDetails(renderer, GetSpaceRect(), transformer, GetOrienation(), selected);
}

MI::ClickEvent node::BlockObject::OnLMBDown(MouseButtonEvent& e)
{
    return GraphicsObject::OnLMBDown(e);
}

void node::BlockObject::OnSceneChange()
{
    auto&& scene = GetScene();
    for (auto&& socket : m_sockets)
    {
        socket->SetScene(scene);
    }
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

node::BlockSocketObject* node::BlockObject::GetSocketById(model::SocketId id)
{
    auto it = std::find_if(m_sockets.begin(), m_sockets.end(), [&](const std::unique_ptr<BlockSocketObject>& sock) {return id == sock->GetId(); });
    assert(it != m_sockets.end());
    if (it != m_sockets.end())
    {
        return it->get();
    }
    return nullptr;
}

void node::BlockObject::UpdateStyler(const model::BlockDataCRef& model)
{
    m_styler->UpdateProperties(model);
}

void node::BlockObject::RenewSockets(std::span<const model::BlockSocketModel> new_sockets)
{
    // disconnect connected nodes
    for (auto&& socket : GetSockets())
    {
        auto&& connected_node = socket->GetConnectedNode();
        if (connected_node)
        {
            connected_node->SetConnectedSocket(nullptr);
        }
    }
    m_sockets.clear();

    // add new ones
    for (const auto& socket : new_sockets)
    {
        auto socket_ptr = std::make_unique<BlockSocketObject>(socket.GetType(), socket.GetId(),
            socket.GetPosition(), GetScene(), this);
        AddSocket(std::move(socket_ptr));
    }
}

void node::BlockObject::SetResizeHandles(BlockResizeObject& resize_object)
{
    m_resizer = resize_object.GetMIHandlePtr();
}

void node::BlockObject::HideResizeHandles()
{
    m_resizer = nullptr;
}


void node::BlockObject::OnSetSpaceRect(const model::Rect& rect)
{
    GraphicsObject::OnSetSpaceRect(rect);
    if (m_resizer)
    {
        m_resizer->SetSpaceRect(BlockResizeObject::RectForBlockRect(rect));
    }
    RePositionSockets();
}

void node::BlockObject::RePositionSockets()
{
    auto&& block_rect = GetSpaceRect();
    auto&& origin = model::Util::get_rect_origin(block_rect);
    for (auto&& socket : m_sockets)
    {
        const auto& sock_pos = socket->GetCenterInBlock();
        socket->SetCenterInSpace(sock_pos + origin);
    }
}

node::GraphicsObject* node::BlockObject::OnGetInteractableAtPoint(const model::Point& point)
{
    for (auto&& sock : m_sockets)
    {
        node::GraphicsObject* hover = sock->GetInteractableAtPoint(point);
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

