#include "GraphicsObjectsManager.hpp"

#include "NodeSDLStylers/BlockStyler.hpp"

#include "GraphicsScene/GraphicsScene.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "GraphicsLogic/GraphicsLogic.hpp"

#include "NodeSDLStylers/BlockStylerFactory.hpp"

node::GraphicsObjectsManager::GraphicsObjectsManager(GraphicsScene& scene, std::shared_ptr<BlockStylerFactory> styler_factory)
    :m_scene{scene.GetMIHandlePtr()}, m_blockStylerFactory{std::move(styler_factory)}
{
    assert(GetGraphicsScene());
    assert(m_blockStylerFactory);
}

void node::GraphicsObjectsManager::SetSceneModel(std::shared_ptr<SceneModelManager> scene)
{
    m_blocks.clear();
    m_net_nodes.clear();
    m_net_segments.clear();
    GetGraphicsScene()->ClearAllObjects();
    GetGraphicsScene()->SetGraphicsLogic(nullptr);
    if (m_sceneModel)
    {
        m_sceneModel->Detach(*this);
    }
    m_sceneModel = std::move(scene);
    m_sceneModel->Attach(*this);
    for (auto&& block : m_sceneModel->GetBlocks())
    {
        auto styler = m_blockStylerFactory->GetStyler(block.GetStyler(), block);
        styler->PositionSockets(block.GetSockets(), block.GetBounds(), block.GetOrienation());
        std::unique_ptr<node::BlockObject> obj = node::BlockObject::Create(GetGraphicsScene(), block, std::move(styler));
        auto ptr = obj.get();
        GetGraphicsScene()->AddObject(std::move(obj), GraphicsScene::BlockLayer);
        m_blocks.emplace(block.GetId(), ptr);
    }
    for (const auto& net_node : m_sceneModel->GetModel().GetNetNodes())
    {
        auto node = std::make_unique<node::NetNode>(net_node.GetPosition(), GetGraphicsScene());
        node->SetId(net_node.GetId());
        m_net_nodes.emplace(net_node.GetId(), node.get());
        GetGraphicsScene()->AddObject(std::move(node), GraphicsScene::NetNodeLayer);
    }
    for (const auto& net_segment : m_sceneModel->GetModel().GetNetSegments())
    {
        auto it_node1 = m_net_nodes.find(net_segment.m_firstNodeId);
        auto it_node2 = m_net_nodes.find(net_segment.m_secondNodeId);
        assert(it_node1 != m_net_nodes.end());
        assert(it_node2 != m_net_nodes.end());
        if (it_node1 == m_net_nodes.end() || it_node2 == m_net_nodes.end())
        {
            SDL_Log("broken scene!");
            return;
        }
        auto segment_obj = std::make_unique<node::NetSegment>(net_segment.m_orientation, it_node1->second, it_node2->second, GetGraphicsScene());

        segment_obj->SetId(net_segment.GetId());
        m_net_segments.emplace(net_segment.GetId(), segment_obj.get());
        GetGraphicsScene()->AddObject(std::move(segment_obj), GraphicsScene::SegmentLayer);
    }
    for (const auto& socket_connection : m_sceneModel->GetModel().GetSocketConnections())
    {
        auto it_block = m_blocks.find(socket_connection.socketId.block_id);
        assert(it_block != m_blocks.end());
        auto it_node = m_net_nodes.find(socket_connection.NodeId);
        assert(it_node != m_net_nodes.end());
        if (it_block == m_blocks.end() || it_node == m_net_nodes.end())
        {
            SDL_Log("broken scene conn!");
            return;
        }
        auto&& socket = it_block->second->GetSocketById(socket_connection.socketId.socket_id);
        assert(socket);
        if (!socket)
        {
            SDL_Log("broken scene socket!");
            return;
        }
        socket->get().SetConnectedNode(it_node->second);
    }
}

void node::GraphicsObjectsManager::OnNotify(SceneModification& e)
{
    switch (e.type)
    {
    case SceneModification::type_t::BlockAdded:
    {
        auto& model_ref = std::get<model::BlockModelConstRef>(e.data);
        auto styler = m_blockStylerFactory->GetStyler(model_ref.get().GetStyler(), model_ref);
        std::unique_ptr<node::BlockObject> obj = node::BlockObject::Create(GetGraphicsScene(), model_ref, std::move(styler));
        auto* ptr = obj.get();
        GetGraphicsScene()->AddObject(std::move(obj), GraphicsScene::BlockLayer);
        m_blocks.emplace(model_ref.get().GetId(), ptr);
        GetGraphicsScene()->ClearCurrentSelection();
        GetGraphicsScene()->AddSelection(ptr->GetFocusHandlePtr());
        break;
    }
    case SceneModification::type_t::BlockAddedWithConnections:
    {
        auto& report = std::get<BlockAddWithConnectionsReport>(e.data);
        auto& model_ref = report.block;
        auto styler = m_blockStylerFactory->GetStyler(model_ref.get().GetStyler(), model_ref);
        std::unique_ptr<node::BlockObject> obj = node::BlockObject::Create(GetGraphicsScene(), model_ref, std::move(styler));
        auto* ptr = obj.get();
        GetGraphicsScene()->AddObject(std::move(obj), GraphicsScene::BlockLayer);
        m_blocks.emplace(model_ref.get().GetId(), ptr);
        for (auto& conn : report.connections)
        {
            auto socket_optional = ptr->GetSocketById(conn.socketId.socket_id);
            assert(socket_optional);
            if (!socket_optional)
            {
                continue;
            }
            auto node_it = m_net_nodes.find(conn.NodeId);
            assert(node_it != m_net_nodes.end());
            if (node_it == m_net_nodes.end())
            {
                continue;
            }
            node_it->second->SetConnectedSocket(&(socket_optional->get()));
        }
        GetGraphicsScene()->ClearCurrentSelection();
        GetGraphicsScene()->AddSelection(ptr->GetFocusHandlePtr());
        break;
    }
    case SceneModificationType::BlockRemoved:
    {
        auto model_id = std::get<model::BlockId>(e.data);        
        auto it = m_blocks.find(model_id);
        if (it != m_blocks.end())
        {
            for (auto&& socket : it->second->GetSockets())
            {
                auto&& connected_node = socket->GetConnectedNode();
                if (connected_node)
                {
                    connected_node->SetConnectedSocket(nullptr);
                }
            }
            GetGraphicsScene()->PopObject(it->second);
            m_blocks.erase(it);
        }
        break;
    }
    case SceneModificationType::BlockMoved:
    {
        auto model_id = std::get<model::BlockModelConstRef>(e.data).get().GetId();
        auto new_position = std::get<model::BlockModelConstRef>(e.data).get().GetPosition();
        auto it = m_blocks.find(model_id);
        if (it != m_blocks.end())
        {
            it->second->SetSpaceOrigin(new_position);
        }
        break;
    }
    case SceneModificationType::BlockResized:
    {
        auto&& block_ref = std::get<model::BlockModelConstRef>(e.data).get();
        auto model_id = block_ref.GetId();
        auto new_bounds = block_ref.GetBounds();
        auto it = m_blocks.find(model_id);
        if (it != m_blocks.end())
        {
            for (auto&& socket : it->second->GetSockets())
            {
                assert(socket->GetId());
                auto&& block_socket = block_ref.GetSocketById(*socket->GetId());
                socket->SetCenterInBlock(block_socket->get().GetPosition());
            }
            it->second->SetOrientation(block_ref.GetOrienation());
            it->second->SetSpaceRect(new_bounds);
        }
        break;
    }
    case SceneModificationType::BlockPropertiesModified:
    {
        auto model = std::get<model::BlockModelConstRef>(e.data);
        auto it = m_blocks.find(model.get().GetId());
        assert(it != m_blocks.end());
        if (it != m_blocks.end())
        {
            it->second->UpdateStyler(model);
        }
        break;
    }
    case SceneModificationType::BlockPropertiesAndSocketsModified:
    {
        auto model = std::get<model::BlockModelConstRef>(e.data);
        auto it = m_blocks.find(model.get().GetId());
        assert(it != m_blocks.end());
        if (it != m_blocks.end())
        {
            it->second->RenewSockets(model.get().GetSockets());
            it->second->UpdateStyler(model);
        }
        break;
    }
    case SceneModificationType::NetUpdated:
    {
        HandleNetUpdate(std::get<std::reference_wrapper<NetModificationReport>>(e.data));
        break;
    }
    }
}

void node::GraphicsObjectsManager::OnNotify(BlockObjectDropped& object)
{
    model::BlockModel block{ model::BlockModel{ object.object.block } };
    if (block.GetClass() == "SubSystem")
    {
        m_parent_manager->AddNewSubSceneToScene(block, m_sceneModel->GetSubSceneId());
    }
    model::Rect bounds = object.object.block.GetBounds();
    model::Point offset = { -bounds.w / 2, -bounds.h / 2 };
    block.SetPosition(GetGraphicsScene()->QuantizePoint(GetGraphicsScene()->GetSpaceScreenTransformer().ScreenToSpacePoint(object.p) + offset));
    m_sceneModel->AddNewBlock(std::move(block));
}

void node::GraphicsObjectsManager::HandleNetUpdate(NetModificationReport& report)
{

    // handle remove connections
    for (const auto& removed_conn : report.removed_connections)
    {
        auto it = m_blocks.find(removed_conn.block_id);
        assert(it != m_blocks.end());
        if (it != m_blocks.end())
        {
            auto& block = *it->second;
            for (auto&& socket : block.GetSockets())
            {
                if (socket->GetId() == removed_conn.socket_id)
                {
                    auto* connecteed_node = socket->GetConnectedNode();
                    connecteed_node->SetConnectedSocket(nullptr);
                    break;
                }
            }
        }
    }

    // handle removed segments
    for (const auto& removed_segement : report.removed_segments)
    {
        auto it = m_net_segments.find(removed_segement);
        assert(it != m_net_segments.end());
        if (it != m_net_segments.end())
        {
            auto& segment = *it->second;
            segment.Disconnect();
            m_net_segments.erase(it);
            GetGraphicsScene()->PopObject(&segment);
        }
    }

    // handle removed nodes
    for (const auto& removed_node : report.removed_nodes)
    {
        auto it = m_net_nodes.find(removed_node);
        assert(it != m_net_nodes.end());
        if (it != m_net_nodes.end())
        {
            auto& node = *it->second;
            node.SetConnectedSocket(nullptr);
            m_net_nodes.erase(it);
            GetGraphicsScene()->PopObject(&node);
        }
    }

    // handle added nodes
    for (const auto& added_node : report.added_nodes)
    {
        auto node = std::make_unique<NetNode>(added_node.get().GetPosition(), GetGraphicsScene());
        m_net_nodes.emplace(added_node.get().GetId(), node.get());
        node->SetId(added_node.get().GetId());
        GetGraphicsScene()->AddObject(std::move(node), GraphicsScene::NetNodeLayer);
    }

    // handle update nodes
    for (const auto& updated_node : report.update_nodes)
    {
        auto it = m_net_nodes.find(updated_node.node_id);
        assert(it != m_net_nodes.end());
        if (it != m_net_nodes.end())
        {
            it->second->setCenter(updated_node.new_position);
            it->second->UpdateConnectedSegments();
        }
    }

    // handle update segments
    for (const auto& update_segment : report.update_segments)
    {
        auto it = m_net_segments.find(update_segment.get().GetId());
        assert(it != m_net_segments.end());
        if (it != m_net_segments.end())
        {
            it->second->Disconnect();
            auto it1 = m_net_nodes.find(update_segment.get().m_firstNodeId);
            assert(it1 != m_net_nodes.end());
            if (it1 == m_net_nodes.end())
            {
                continue;
            }
            auto it2 = m_net_nodes.find(update_segment.get().m_secondNodeId);
            assert(it2 != m_net_nodes.end());
            if (it2 == m_net_nodes.end())
            {
                continue;
            }
            it->second->Connect(it1->second, it2->second, update_segment.get().m_orientation);
        }
    }

    // handle added segments
    for (const auto& added_segment : report.added_segments)
    {
        auto it1 = m_net_nodes.find(added_segment.get().m_firstNodeId);
        assert(it1 != m_net_nodes.end());
        if (it1 == m_net_nodes.end())
        {
            continue;
        }
        auto it2 = m_net_nodes.find(added_segment.get().m_secondNodeId);
        assert(it2 != m_net_nodes.end());
        if (it2 == m_net_nodes.end())
        {
            continue;
        }
        auto segment = std::make_unique<NetSegment>(added_segment.get().m_orientation, it1->second, it2->second, GetGraphicsScene());
        auto id = added_segment.get().GetId();
        segment->SetId(id);
        m_net_segments.emplace(id, segment.get());
        GetGraphicsScene()->AddObject(std::move(segment), GraphicsScene::SegmentLayer);
    }

    // handle added connections
    for (const auto& added_conn : report.added_connections)
    {
        auto& conn = added_conn.get();
        auto it = m_blocks.find(conn.socketId.block_id);
        assert(it != m_blocks.end());
        if (it == m_blocks.end())
        {
            continue;
        }

        for (auto& socket : it->second->GetSockets())
        {
            if (socket->GetId() && (*socket->GetId()) == added_conn.get().socketId.socket_id)
            {
                auto it2 = m_net_nodes.find(added_conn.get().NodeId);
                assert(it2 != m_net_nodes.end());
                if (it2 == m_net_nodes.end())
                {
                    break;
                }
                socket->SetConnectedNode(it2->second);
            }
        }
    }

}
