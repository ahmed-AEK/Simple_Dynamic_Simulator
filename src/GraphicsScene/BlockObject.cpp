#include "BlockObject.hpp"
#include "BlockSocketObject.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "GraphicsScene/GraphicsScene.hpp"
#include "GraphicsScene/BlockResizeObject.hpp"

std::unique_ptr<node::BlockObject> node::BlockObject::Create(const model::BlockModel& model, std::unique_ptr<BlockStyler> styler)
{
    auto&& model_bounds = model.GetBounds();
    auto ptr = std::make_unique<BlockObject>(model::ObjectSize{ model_bounds.w, model_bounds.h }, std::move(styler), model.GetId(), model.GetOrienation());
    ptr->SetPosition({ model_bounds.x, model_bounds.y });
    for (const auto& socket : model.GetSockets())
    {
        auto socket_ptr = std::make_unique<BlockSocketObject>(socket.GetType(), socket.GetId(),
            socket.GetPosition(), socket.GetConnectionSide(), socket.GetCategory());
        ptr->AddSocket(std::move(socket_ptr));
    }
    return ptr;
}

node::BlockObject::BlockObject(const model::ObjectSize& size,
    std::unique_ptr<BlockStyler> styler, std::optional<model::BlockId> model_id, model::BlockOrientation orientation)
    :GraphicsObject{ size, ObjectType::block, nullptr }, m_id{ model_id }, m_styler{ std::move(styler) }, m_orientation{ orientation }
{
}

node::BlockObject::~BlockObject()
{
}

void node::BlockObject::Draw(SDL::Renderer& renderer, const SpaceScreenTransformer& transformer)
{
    assert(m_styler);
    auto selected = IsSelected();
    auto rect = GetSize().ToRect();
    m_styler->DrawBlockOutline(renderer, rect, transformer, GetOrienation(), selected);
    m_styler->DrawBlockDetails(renderer, rect, transformer, GetOrienation(), selected);
    for (const auto& socket : m_sockets)
    {
        m_styler->DrawBlockSocket(renderer, socket->GetCenterInBlock(), transformer, socket->GetSocketType());
    }
}

MI::ClickEvent node::BlockObject::OnLMBDown(MouseButtonEvent& e)
{
    return GraphicsObject::OnLMBDown(e);
}

void node::BlockObject::OnSetPosition(const model::Point& position)
{
    GraphicsObject::OnSetPosition(position);
    if (auto attachment = GetAttachment())
    {
        attachment->OnObjectRectUpdate(GetSceneRect());
    }
}

void node::BlockObject::OnSetSize(const model::ObjectSize& size)
{
    GraphicsObject::OnSetSize(size);
    if (auto attachment = GetAttachment())
    {
        attachment->OnObjectRectUpdate(GetSceneRect());
    }
}

void node::BlockObject::AddSocket(std::unique_ptr<BlockSocketObject> socket)
{
    m_sockets.push_back(std::move(socket));
    m_sockets.back()->SetParent(this);
    m_sockets.back()->SetParentBlock(this);
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
            socket.GetPosition(), socket.GetConnectionSide(), socket.GetCategory());
        AddSocket(std::move(socket_ptr));
    }
}

node::GraphicsObject* node::BlockObject::OnGetInteractableAtPoint(const model::Point& point)
{
    for (auto&& sock : m_sockets)
    {
        node::GraphicsObject* hover = sock->GetInteractableAtPoint(point - sock->GetPosition());
        if (hover)
        {
            return hover;
        }
    }
    return this;
}

