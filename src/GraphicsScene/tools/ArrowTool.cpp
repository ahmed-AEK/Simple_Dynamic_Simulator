#include "GraphicsScene/tools/ArrowTool.hpp"
#include "GraphicsScene/GraphicsObject.hpp"
#include <cassert>
#include "GraphicsScene/GraphicsScene.hpp"

MI::ClickEvent node::ArrowTool::OnLMBDown(const model::Point& p)
{
    node::GraphicsObject* current_hover = GetScene()->GetCurrentHover();
    if (current_hover)
    {
        // selection code
        InternalSelectObject(current_hover);

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
        m_isDragging = true;
        m_capturing_mouse = true;
        m_startPointScreen = GetScene()->GetSpaceScreenTransformer().SpaceToScreenPoint(p);
        auto&& space_rect = scene->GetSpaceRect();
        m_startEdgeSpace = { space_rect.x, space_rect.y};
        return MI::ClickEvent::CAPTURE_START;
    }
    return MI::ClickEvent::NONE;
}

MI::ClickEvent node::ArrowTool::OnLMBUp(const model::Point& p)
{
    UNUSED_PARAM(p);
    if (m_isDragging)
    {
        m_capturing_mouse = false;
        m_isDragging = false;
        return MI::ClickEvent::CAPTURE_END;
    }
	return MI::ClickEvent();
}

void node::ArrowTool::OnMouseMove(const model::Point& p)
{
    
    if (m_isDragging)
    {
        GraphicsScene* scene = GetScene();
        assert(scene);
        auto&& transformer = scene->GetSpaceScreenTransformer();
        SDL_Point point = transformer.SpaceToScreenPoint(p);
        const auto current_position_difference_space_vector = transformer.ScreenToSpaceVector({
        point.x - m_startPointScreen.x, point.y - m_startPointScreen.y
            });

        auto&& space_rect = scene->GetSpaceRect();
        scene->SetSpaceRect({
            m_startEdgeSpace.x - current_position_difference_space_vector.x,
            m_startEdgeSpace.y - current_position_difference_space_vector.y,
            space_rect.w,
            space_rect.h
            });
        scene->UpdateObjectsRect();
        scene->InvalidateRect();
    }
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