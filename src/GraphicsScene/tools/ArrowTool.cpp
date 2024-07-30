#include "GraphicsScene/tools/ArrowTool.hpp"
#include "GraphicsScene/GraphicsObject.hpp"
#include <cassert>
#include "GraphicsScene/GraphicsScene.hpp"
#include "GraphicsLogic/ScreenDragLogic.hpp"
#include "GraphicsLogic/BlockDragLogic.hpp"
#include "BlockObject.hpp"

MI::ClickEvent node::ArrowTool::OnLMBDown(const model::Point& p)
{
    node::GraphicsObject* current_hover = GetScene()->GetCurrentHover();
    if (current_hover)
    {
        // selection code
        InternalSelectObject(current_hover);

        if (current_hover->GetObjectType() == ObjectType::block)
        {
            auto block_obj = static_cast<BlockObject*>(current_hover);
            auto obj_rect = current_hover->GetSpaceRect();
            auto drag_logic = std::make_unique<logic::BlockDragLogic>(p, model::Point{ obj_rect.x, obj_rect.y }, *block_obj, GetScene());
            GetScene()->SetGraphicsLogic(std::move(drag_logic));
            return MI::ClickEvent::CLICKED;
        }
        /*
        // do Click
        MI::ClickEvent result = current_hover->LMBDown(p);
        switch (result)
        {
            using enum MI::ClickEvent;
        case CAPTURE_START:
        {
            m_mouse_capture_mode = node::GraphicsScene::CAPTURE_MODE::OBJECT;
            return CAPTURE_START;
            break;
        }
        case CAPTURE_END:
        {
            m_mouse_capture_mode = node::GraphicsScene::CAPTURE_MODE::NONE;
            return CAPTURE_END;
            break;
        }
        case CLICKED:
        {
            return CLICKED;
            break;
        }
        case NONE:
        {
            break;
        }
        }

        // do drag
        m_drag_objects.clear();
        for (auto&& object_ptr : m_current_selection)
        {
            node::GraphicsObject* object = object_ptr.GetObjectPtr();
            if (object != nullptr && object->isDraggable())
            {
                m_drag_objects.emplace_back(object->GetMIHandlePtr(), SDL_Point{ object->GetSpaceRect().x, object->GetSpaceRect().y });
            }
        }
        if (m_drag_objects.size() != 0)
        {
            m_mouse_capture_mode = node::GraphicsScene::CAPTURE_MODE::OBJECTS_DRAG;
            m_StartPointScreen = p;
            return MI::ClickEvent::CAPTURE_START;
        }
        */
    }
    else
    {
        GraphicsScene* scene = GetScene();
        assert(scene);
        scene->ClearCurrentSelection();
        auto startPointScreen = GetScene()->GetSpaceScreenTransformer().SpaceToScreenPoint(p);
        auto&& space_rect = scene->GetSpaceRect();
        auto startEdgeSpace = model::Point{ space_rect.x, space_rect.y};
        auto logic = std::make_unique<logic::ScreenDragLogic>(startPointScreen, startEdgeSpace, GetScene());
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