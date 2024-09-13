#include "GraphicsObjectsManager.hpp"

#include "NodeSDLStylers/BlockStyler.hpp"

#include "GraphicsScene/GraphicsScene.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "GraphicsLogic/GraphicsLogic.hpp"

#include "NodeSDLStylers/BlockStylerFactory.hpp"

node::GraphicsObjectsManager::GraphicsObjectsManager(GraphicsScene& scene, std::shared_ptr<BlockStylerFactory> styler_factory)
    :m_scene{&scene}, m_blockStylerFactory{std::move(styler_factory)}
{
    assert(GetScene());
    assert(m_blockStylerFactory);
}

void node::GraphicsObjectsManager::SetSceneModel(std::shared_ptr<SceneModelManager> scene)
{
    m_blocks.clear();
    m_net_nodes.clear();
    m_net_segments.clear();
    GetScene()->ClearAllObjects();
    GetScene()->SetGraphicsLogic(nullptr);
    if (m_sceneModel)
    {
        m_sceneModel->Detach(*this);
    }
    m_sceneModel = std::move(scene);
    m_sceneModel->Attach(*this);
    for (auto&& block : m_sceneModel->GetBlocks())
    {
        auto styler = m_blockStylerFactory->GetStyler(block.GetStyler(), block.GetStylerProperties());
        styler->PositionNodes(block);
        std::unique_ptr<node::BlockObject> obj = node::BlockObject::Create(GetScene(), block, std::move(styler));
        auto ptr = obj.get();
        GetScene()->AddObject(std::move(obj), GraphicsScene::BlockLayer);
        m_blocks.emplace(block.GetId(), ptr);
    }
}

void node::GraphicsObjectsManager::OnNotify(SceneModification& e)
{
    switch (e.type)
    {
    case SceneModification::type_t::BlockAdded:
    {
        auto& model_ref = std::get<model::BlockModelRef>(e.data);
        auto styler = m_blockStylerFactory->GetStyler(model_ref.get().GetStyler(), model_ref.get().GetStylerProperties());
        std::unique_ptr<node::BlockObject> obj = node::BlockObject::Create(GetScene(), model_ref, std::move(styler));
        auto* ptr = obj.get();
        GetScene()->AddObject(std::move(obj), GraphicsScene::BlockLayer);
        m_blocks.emplace(model_ref.get().GetId(), ptr);
        GetScene()->ClearCurrentSelection();
        GetScene()->AddSelection(ptr->GetFocusHandlePtr());
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
            GetScene()->PopObject(it->second);
            m_blocks.erase(it);
        }
        break;
    }
    case SceneModificationType::BlockMoved:
    {
        auto model_id = std::get<model::BlockModelRef>(e.data).get().GetId();
        auto new_position = std::get<model::BlockModelRef>(e.data).get().GetPosition();
        auto it = m_blocks.find(model_id);
        if (it != m_blocks.end())
        {
            it->second->SetSpaceOrigin(new_position);
        }
        break;
    }
    /*
    case SceneModificationType::NetAdded:
    {
        auto net_ref = std::get<model::NetModelRef>(e.data);
        auto net_id = net_ref.get().GetId();

        std::vector<NetNode*> nodes;
        for (auto&& node : net_ref.get().GetNetNodes())
        {
            auto obj = std::make_unique<NetNode>(node.GetPosition(), GetScene());
            obj->SetId(model::NetNodeUniqueId{ node.GetId(), net_id });
            auto ptr = obj.get();
            nodes.push_back(ptr);
            GetScene()->AddObject(std::move(obj), GraphicsScene::NetNodeLayer);
            m_net_nodes.emplace(model::NetNodeUniqueId{ node.GetId(), net_id}, ptr);
        }
        for (auto&& segment : net_ref.get().GetNetSegments())
        {
            auto orientation = segment.m_orientation;

            auto start_node = std::find_if(nodes.begin(), nodes.end(),
                [&](NetNode* node) {
                    assert(node->GetId());
                    return node->GetId()->node_id == segment.m_firstNodeId;
                });
            NetNode* start_node_ptr = start_node == nodes.end() ? nullptr : *start_node;
            auto end_node = std::find_if(nodes.begin(), nodes.end(),
                [&](NetNode* node) {
                    assert(node->GetId());
                    return node->GetId()->node_id == segment.m_secondNodeId;
                });
            NetNode* end_node_ptr = end_node == nodes.end() ? nullptr : *end_node;

            auto obj = std::make_unique<NetSegment>(orientation, start_node_ptr, end_node_ptr, GetScene());
            auto ptr = obj.get();
            obj->SetId(model::NetSegmentUniqueId{ segment.GetId(), net_id });
            GetScene()->AddObject(std::move(obj), GraphicsScene::SegmentLayer);
            m_net_segments.emplace(model::NetSegmentUniqueId{ segment.GetId(), net_id }, ptr);
        }
        for (auto&& conn : net_ref.get().GetSocketConnections())
        {
            auto it = m_blocks.find(conn.socketId.block_id);
            assert(it != m_blocks.end()); // attaching node to socket that doesn't exist !
            if (it == m_blocks.end())
            {
                continue; // likely bug, but don't crash
            }

            for (auto&& sock : it->second->GetSockets())
            {
                if (sock->GetId() == conn.socketId.socket_id)
                {
                    auto connected_node = std::find_if(nodes.begin(), nodes.end(),
                        [&](NetNode* node) {
                            assert(node->GetId());
                            return node->GetId()->node_id == conn.NodeId;
                        });
                    sock->SetConnectedNode(*connected_node);
                    break;
                }
            }
        }
        break;
    }
    case SceneModificationType::LeafNodeMoved:
    {
        auto report = std::get<LeafNodeMovedReport>(e.data);
        auto it = m_net_nodes.find(report.moved_node);
        assert(it != m_net_nodes.end());
        if (it == m_net_nodes.end())
        {
            return;
        }

        auto it2 = m_net_nodes.find(report.moved_node);
        assert(it2 != m_net_nodes.end());
        if (it2 == m_net_nodes.end())
        {
            return;
        }

        auto* node_obj = it->second;
        auto* connectd_node_obj = it2->second;
        auto conn = report.new_socket;
        if (!conn)
        {
            node_obj->SetConnectedSocket(nullptr);
        }
        else
        {
            auto it_block = m_blocks.find(conn->block_id);
            assert(it_block != m_blocks.end());
            if (it_block != m_blocks.end())
            {
                for (auto&& socket : it_block->second->GetSockets())
                {
                    if (socket->GetId() == conn->socket_id)
                    {
                        node_obj->SetConnectedSocket(socket.get());
                        break;
                    }
                }
            }
        }
        node_obj->setCenter(report.new_position);
        connectd_node_obj->setCenter({ connectd_node_obj->getCenter().x, report.new_position.y });
        connectd_node_obj->UpdateConnectedSegments();
        break;
    }
    */
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

    model::Rect bounds = object.object.block.GetBounds();
    model::Point offset = { -bounds.w / 2, -bounds.h / 2 };
    block.SetPosition(GetScene()->QuantizePoint(GetScene()->GetSpaceScreenTransformer().ScreenToSpacePoint(object.p) + offset));
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
            GetScene()->PopObject(&segment);
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
            GetScene()->PopObject(&node);
        }
    }

    // handle added nodes
    for (const auto& added_node : report.added_nodes)
    {
        auto node = std::make_unique<NetNode>(added_node.get().GetPosition(), GetScene());
        m_net_nodes.emplace(added_node.get().GetId(), node.get());
        node->SetId(added_node.get().GetId());
        GetScene()->AddObject(std::move(node), GraphicsScene::NetNodeLayer);
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
        auto segment = std::make_unique<NetSegment>(added_segment.get().m_orientation, it1->second, it2->second, GetScene());
        auto id = added_segment.get().GetId();
        segment->SetId(id);
        m_net_segments.emplace(id, segment.get());
        GetScene()->AddObject(std::move(segment), GraphicsScene::SegmentLayer);
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
