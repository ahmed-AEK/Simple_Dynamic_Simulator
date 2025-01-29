#include "GraphicsScene/tools/ArrowTool.hpp"
#include "GraphicsScene/GraphicsObject.hpp"
#include "GraphicsScene/GraphicsScene.hpp"
#include "GraphicsLogic/ScreenDragLogic.hpp"
#include "GraphicsLogic/BlockDragLogic.hpp"
#include "GraphicsLogic/NewNetLogic.hpp"
#include "GraphicsLogic/LeafNetNodeDragLogic.hpp"
#include "BlockObject.hpp"
#include "BlockSocketObject.hpp"
#include "NetObject.hpp"
#include "GraphicsLogic/MiddleSegmentDragLogic.hpp"
#include "BlockResizeObject.hpp"


MI::ClickEvent node::ArrowTool::OnLMBDown(MouseButtonEvent& e, GraphicsScene& scene, GraphicsObjectsManager& manager)
{
    node::GraphicsObject* current_hover = scene.GetCurrentHover();

    if (!current_hover || current_hover->GetObjectType() != ObjectType::interactive)
    {
        if (m_current_block_resize_slot.object && m_current_block_resize_slot.scene)
        {
            m_current_block_resize_slot.scene->PopObject(
                m_current_block_resize_slot.object.GetObjectPtr());
            m_current_block_resize_slot = ResizerObjectSlot{};
        }
    }

    if (current_hover)
    {
        // selection code
        InternalSelectObject(current_hover, scene);

        switch (current_hover->GetObjectType())
        {
        case ObjectType::block:
        {
            if (e.double_click && m_last_clicked_block.GetObjectPtr() == current_hover)
            {
                SDL_Log("Double Click Started!");
                BlockDoubleClickedEvent ev{ static_cast<BlockObject*>(current_hover) };
                Notify(ev);
                return MI::ClickEvent::CLICKED;
            }
            else
            {
                scene.BumpObjectInLayer(current_hover);
                m_last_clicked_block = current_hover->GetFocusHandlePtr();
            }

            auto* block_obj = static_cast<BlockObject*>(current_hover);

            if (m_current_block_resize_slot.object && m_current_block_resize_slot.scene)
            {
                m_current_block_resize_slot.scene->PopObject(
                    m_current_block_resize_slot.object.GetObjectPtr());
                m_current_block_resize_slot = ResizerObjectSlot{};
            }

            auto resizer = CreateResizeObject(*block_obj, manager);
            if (resizer)
            {
                m_current_block_resize_slot = ResizerObjectSlot{ HandlePtrS<GraphicsScene,Widget>{scene}, resizer->GetMIHandlePtr() };
                scene.AddObject(std::move(resizer), GraphicsScene::InteractiveLayer);
            }

            auto drag_logic = logic::BlockDragLogic::TryCreate(e.point, 
                *block_obj, &scene, &manager);
            scene.SetGraphicsLogic(std::move(drag_logic));
            return MI::ClickEvent::CAPTURE_START;
        }
        case ObjectType::socket:
        {
            auto* socket = static_cast<BlockSocketObject*>(current_hover);
            scene.BumpObjectInLayer(socket->GetParentBlock());
            if (!socket->GetConnectedNode())
            {
                auto new_logic = logic::NewNetLogic::CreateFromSocket(*socket, &scene, &manager);
                if (new_logic)
                {
                    scene.SetGraphicsLogic(std::move(new_logic));
                    return MI::ClickEvent::CAPTURE_START;
                }
            }
            break;
        }
        case ObjectType::netNode:
        {
            auto* node = static_cast<NetNode*>(current_hover);
            if (node->GetConnectedSegmentsCount() == 1)
            {
                if (auto ptr = logic::LeafNetNodeDragLogic::TryCreate(*node, scene, manager))
                {
                    scene.SetGraphicsLogic(std::move(ptr));
                    return MI::ClickEvent::CAPTURE_START;
                }
            }
            break;
        }
        case ObjectType::netSegment:
        {
            auto* segment = static_cast<NetSegment*>(current_hover);
            scene.BumpObjectInLayer(current_hover);
            if (auto ptr = logic::MiddleSegmentDragLogic::Create(*segment->getStartNode(),
                *segment->getEndNode(), *segment, e.point, &scene, &manager))
            {
                scene.SetGraphicsLogic(std::move(ptr));
                return MI::ClickEvent::CAPTURE_START;
            }
            break;
        }
        case ObjectType::interactive:
        {
            return current_hover->LMBDown({ e.point - current_hover->GetScenePosition() });
            break;
        }
        default: break;
        }
        
    }
    else
    {
        scene.ClearCurrentSelection();
        auto startPointScreen = scene.GetSpaceScreenTransformer().SpaceToScreenPoint(e.point);
        auto&& space_rect = scene.GetSpaceRect();
        auto startEdgeSpace = model::Point{ space_rect.x, space_rect.y};
        auto logic = std::make_unique<logic::ScreenDragLogic>(startPointScreen, startEdgeSpace, &scene, &manager);
        scene.SetGraphicsLogic(std::move(logic));
        return MI::ClickEvent::CAPTURE_START;
    }
    return MI::ClickEvent::NONE;
}

bool node::ArrowTool::InternalSelectObject(GraphicsObject* object, GraphicsScene& scene)
{
    if (object->isSelectable())
    {
        if (!scene.IsObjectSelected(*object))
        {
            scene.ClearCurrentSelection();
            if (object->GetObjectType() == ObjectType::netNode)
            {
                AddSelectConnectedNet(*static_cast<NetNode*>(object), scene);
            }
            else if (object->GetObjectType() == ObjectType::netSegment)
            {
                AddSelectConnectedNet(*static_cast<NetSegment*>(object), scene);
            }
            else
            {
                scene.AddSelection(object->GetFocusHandlePtr());
            }
        }
        return true;
    }
    else
    {
        scene.ClearCurrentSelection();
        return false;
    }
}

void node::ArrowTool::OnExit()
{
    if (const auto* resizer = m_current_block_resize_slot.object.GetObjectPtr())
    {
        if (auto* scene = m_current_block_resize_slot.scene.GetObjectPtr())
        {
            scene->PopObject(resizer);
            m_current_block_resize_slot = ResizerObjectSlot{};
        }
    }
}

std::unique_ptr<node::BlockResizeObject> node::ArrowTool::CreateResizeObject(BlockObject& block, GraphicsObjectsManager& manager)
{
    model::Rect resizer_rect = BlockResizeObject::RectForBlockRect(block.GetSceneRect());

    auto resizer = std::make_unique<BlockResizeObject>(block, &manager, model::ObjectSize{ resizer_rect.w, resizer_rect.h });
    resizer->SetPosition({ resizer_rect.x, resizer_rect.y });
    block.SetResizeHandles(*resizer);
    return resizer;
}
