#include "GraphicsObjectsManager.hpp"

#include "NodeSDLStylers/BlockStyler.hpp"

#include "GraphicsScene/GraphicsScene.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include "GraphicsScene/NetObject.hpp"
#include "GraphicsLogic/GraphicsLogic.hpp"

node::GraphicsObjectsManager::GraphicsObjectsManager(GraphicsScene& scene)
	:m_scene{&scene}
{
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
    auto styler = std::make_shared<node::BlockStyler>();
    for (auto&& block : m_sceneModel->GetBlocks())
    {
        styler->PositionNodes(*block);
        std::unique_ptr<node::BlockObject> obj = std::make_unique<node::BlockObject>(GetScene(), block, styler);
        auto ptr = obj.get();
        GetScene()->AddObject(std::move(obj), 0);
        m_blocks.emplace(block->GetId(), ptr);
    }
}

void node::GraphicsObjectsManager::OnNotify(SceneModification& e)
{
    switch (e.type)
    {
    case SceneModification::type_t::BlockAdded:
    {
        auto styler = std::make_shared<node::BlockStyler>();
        auto& model_ptr = std::get<model::BlockModelPtr>(e.data);
        std::unique_ptr<node::BlockObject> obj = std::make_unique<node::BlockObject>(GetScene(), model_ptr, styler);
        auto* ptr = obj.get();
        GetScene()->AddObject(std::move(obj), 0);
        m_blocks.emplace(model_ptr->GetId(), ptr);
        GetScene()->ClearCurrentSelection();
        GetScene()->AddSelection(ptr->GetFocusHandlePtr());
        break;
    }
    case SceneModificationType::BlockRemoved:
    {
        auto model_id = std::get<model::BlockModelPtr>(e.data)->GetId();

        auto it = m_blocks.find(model_id);
        if (it != m_blocks.end())
        {
            GetScene()->PopObject(it->second);
        }
        break;
    }
    case SceneModificationType::BlockMoved:
    {
        auto model_id = std::get<model::BlockModelPtr>(e.data)->GetId();
        auto new_position = std::get<model::BlockModelPtr>(e.data)->GetPosition();
        auto it = m_blocks.find(model_id);
        if (it != m_blocks.end())
        {
            it->second->SetSpaceOrigin(new_position);
        }
        break;
    }
    case SceneModificationType::NetAdded:
    {
        auto net_ptr = std::get<model::NetModelPtr>(e.data);
        assert(net_ptr);

        auto net_id = net_ptr->GetId();
        std::vector<NetNode*> nodes;
        for (auto&& node : net_ptr->GetNetNodes())
        {
            auto obj = std::make_unique<NetNode>(node.GetPosition(), GetScene());
            obj->SetId(node.GetId());
            obj->SetNet(net_ptr);
            auto ptr = obj.get();
            nodes.push_back(ptr);
            GetScene()->AddObject(std::move(obj), 200);
            m_net_nodes.emplace(model::NetNodeUniqueId{ node.GetId(), net_id}, ptr);
        }
        for (auto&& segment : net_ptr->GetNetSegments())
        {
            auto orientation = segment.m_orientation == model::NetSegmentModel::NetSegmentOrientation::horizontal ?
                NetOrientation::Horizontal : NetOrientation::Vertical;

            auto start_node = std::find_if(nodes.begin(), nodes.end(),
                [&](NetNode* node) {
                    return node->GetId() == segment.m_firstNodeId;
                });
            NetNode* start_node_ptr = start_node == nodes.end() ? nullptr : *start_node;
            auto end_node = std::find_if(nodes.begin(), nodes.end(),
                [&](NetNode* node) {
                    return node->GetId() == segment.m_secondNodeId;
                });
            NetNode* end_node_ptr = end_node == nodes.end() ? nullptr : *end_node;

            auto obj = std::make_unique<NetSegment>(orientation, start_node_ptr, end_node_ptr, GetScene());
            auto ptr = obj.get();
            obj->SetId(segment.GetId());
            obj->SetNet(net_ptr);
            GetScene()->AddObject(std::move(obj), 100);
            m_net_segments.emplace(model::NetSegmentUniqueId{ segment.GetId(), net_id }, ptr);
        }
        for (auto&& conn : net_ptr->GetSocketConnections())
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
                            return node->GetId() == conn.NodeId;
                        });
                    sock->SetConnectedNode(*connected_node);
                    break;
                }
            }
        }
        break;
    }

    }
}

void node::GraphicsObjectsManager::OnNotify(BlockObjectDropped& object)
{
    auto block = std::make_shared<model::BlockModel>(model::BlockModel{ object.object.block });

    model::Rect bounds = object.object.block.GetBounds();
    model::Point offset = { -bounds.w / 2, -bounds.h / 2 };
    block->SetPosition(GetScene()->QuantizePoint(GetScene()->GetSpaceScreenTransformer().ScreenToSpacePoint(object.p) + offset));

    auto styler = std::make_shared<node::BlockStyler>();
    styler->PositionNodes(*block);
    m_sceneModel->AddNewBlock(block);
}
