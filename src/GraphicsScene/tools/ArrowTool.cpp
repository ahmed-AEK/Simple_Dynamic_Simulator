#include "GraphicsScene/tools/ArrowTool.hpp"
#include "GraphicsScene/GraphicsObject.hpp"
#include <cassert>
#include "GraphicsScene/GraphicsScene.hpp"
#include "GraphicsLogic/ScreenDragLogic.hpp"
#include "GraphicsLogic/BlockDragLogic.hpp"
#include "GraphicsLogic/NewNetLogic.hpp"
#include "GraphicsLogic/LeafNetNodeDragLogic.hpp"
#include "BlockObject.hpp"
#include "BlockSocketObject.hpp"
#include "NetObject.hpp"
#include "GraphicsLogic/VSegmentDragLogic.hpp"


MI::ClickEvent node::ArrowTool::OnLMBDown(const model::Point& p)
{
    node::GraphicsObject* current_hover = GetScene()->GetCurrentHover();
    if (current_hover)
    {
        // selection code
        InternalSelectObject(current_hover);

        switch (current_hover->GetObjectType())
        {
        case ObjectType::block:
        {

            auto* block_obj = static_cast<BlockObject*>(current_hover);
            auto obj_rect = current_hover->GetSpaceRect();
            auto drag_logic = std::make_unique<logic::BlockDragLogic>(p, model::Point{ obj_rect.x, obj_rect.y }, 
                *block_obj, GetScene(), GetObjectsManager());
            GetScene()->SetGraphicsLogic(std::move(drag_logic));
            return MI::ClickEvent::CLICKED;
        }
        case ObjectType::socket:
        {
            auto* socket = static_cast<BlockSocketObject*>(current_hover);
            if (!socket->GetConnectedNode())
            {
                auto new_logic = logic::NewNetLogic::Create(socket, GetScene(), GetObjectsManager());
                if (new_logic)
                {
                    GetScene()->SetGraphicsLogic(std::move(new_logic));
                    return MI::ClickEvent::CLICKED;
                }
            }
            break;
        }
        case ObjectType::netNode:
        {
            auto* node = static_cast<NetNode*>(current_hover);
            if (node->GetConnectedSegmentsCount() == 1)
            {
                assert(GetScene());
                assert(GetObjectsManager());
                if (auto ptr = logic::LeafNetNodeDragLogic::TryCreate(*node, *GetScene(), *GetObjectsManager()))
                {
                    GetScene()->SetGraphicsLogic(std::move(ptr));
                    return MI::ClickEvent::CLICKED;
                }
            }
            break;
        }
        case ObjectType::netSegment:
        {
            auto* segment = static_cast<NetSegment*>(current_hover);
            if (auto ptr = logic::VSegmentDragLogic::Create(*segment->getStartNode(),
                *segment->getEndNode(), *segment, p, GetScene(), GetObjectsManager()))
            {
                GetScene()->SetGraphicsLogic(std::move(ptr));
            }
            break;
        }
        default: break;
        }
        
    }
    else
    {
        GraphicsScene* scene = GetScene();
        assert(scene);
        scene->ClearCurrentSelection();
        auto startPointScreen = GetScene()->GetSpaceScreenTransformer().SpaceToScreenPoint(p);
        auto&& space_rect = scene->GetSpaceRect();
        auto startEdgeSpace = model::Point{ space_rect.x, space_rect.y};
        auto logic = std::make_unique<logic::ScreenDragLogic>(startPointScreen, startEdgeSpace, GetScene(), GetObjectsManager());
        GetScene()->SetGraphicsLogic(std::move(logic));
        return MI::ClickEvent::CLICKED;
    }
    return MI::ClickEvent::NONE;
}

MI::ClickEvent node::ArrowTool::OnLMBUp(const model::Point& p)
{
    UNUSED_PARAM(p);
	return MI::ClickEvent();
}

void node::ArrowTool::OnMouseMove(const model::Point& p)
{
    UNUSED_PARAM(p);
}

bool node::ArrowTool::InternalSelectObject(GraphicsObject* object)
{
    GraphicsScene* scene = GetScene();
    assert(scene);
    if (object->isSelectable())
    {
        if (!scene->IsObjectSelected(*object))
        {
            scene->ClearCurrentSelection();
            scene->AddSelection(object->GetFocusHandlePtr());
        }
        return true;
    }
    else
    {
        scene->ClearCurrentSelection();
        return false;
    }
}