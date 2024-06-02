#include "GraphicsScene.hpp"
#include "toolgui/NodeMacros.h"
#include "GraphicsScene/Node.hpp"
#include "GraphicsScene/NodeSocket.hpp"
#include <algorithm>
#include "GraphicsScene/GraphicsObject.hpp"
#include "GraphicsScene/GraphicsLogic.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/ContextMenu.hpp"

node::GraphicsScene::GraphicsScene(SDL_Rect rect, node::Scene* parent)
:Widget(rect, parent), 
m_spaceRect_base{0, 0, 1000, 1000 * rect.h/rect.w}, 
m_spaceRect{0, 0, 200, 200 * rect.h/rect.w},
m_zoomScale(static_cast<double>(m_spaceRect.w)/m_spaceRect_base.w),
m_spaceScreenTransformer(GetRect(), m_spaceRect)
{
    
}

void node::GraphicsScene::AddObject(std::unique_ptr<node::GraphicsObject> obj, int z_order)
{
    obj->SetSpaceOrigin(
        obj->isAligned() ? QuantizePoint({obj->GetSpaceRect().x, obj->GetSpaceRect().y}):
    SDL_Point{obj->GetSpaceRect().x, obj->GetSpaceRect().y}
    );
    ObjectSlot slot = {std::move(obj), z_order};
    auto iter = std::lower_bound(m_objects.begin(), m_objects.end(), slot, [](const auto& obj1, const auto& obj2) {return obj1.z_order > obj2.z_order;} );
    m_objects.insert(iter, std::move(slot));
}

std::unique_ptr<node::GraphicsObject> node::GraphicsScene::PopObject(const node::GraphicsObject* obj)
{
    auto iter = std::find_if(m_objects.begin(), m_objects.end(), [=](const auto& item) {return item.m_ptr.get() == obj;});
    if (iter != m_objects.end())
    {
        std::unique_ptr<node::GraphicsObject> ret_obj = std::move((*iter).m_ptr);
        m_objects.erase(iter);
        return ret_obj;
    }
    return nullptr;
}

void node::GraphicsScene::Draw(SDL_Renderer *renderer)
{
    for (auto&& it = m_objects.rbegin(); it != m_objects.rend(); it++)
    {
        auto&& object = *it;
        object.m_ptr->Draw(renderer);
    }
}

void node::GraphicsScene::SetCurrentHover(node::GraphicsObject* current_hover)
{
    node::GraphicsObject* old_hover = nullptr;
    if (m_current_mouse_hover.isAlive())
    {
        old_hover = m_current_mouse_hover.GetObjectPtr();
    }
    if (current_hover != old_hover)
    {
        if (old_hover)
        {
            old_hover->MouseOut();
        }
        if (current_hover)
        {
            current_hover->MouseIn();
            m_current_mouse_hover = current_hover->GetMIHandlePtr();
        }
        else
        {
            m_current_mouse_hover = node::HandlePtr<node::GraphicsObject>{};
        }
    }
    if (!m_current_mouse_hover.isAlive())
    {
        m_mouse_capture_mode = CAPTURE_MODE::NONE;
    }
}

node::NodeSocket* node::GraphicsScene::GetSocketAt(const SDL_Point space_point)
{
    for (auto& object : m_objects)
    {
        if (ObjectType::node == object.m_ptr->GetObjectType() && SDL_PointInRect(&space_point, &(object.m_ptr->GetSpaceRect())))
        {
            node::Node* node_pointer = static_cast<node::Node*>(object.m_ptr.get());
            auto&& range = node_pointer->GetSockets();
            auto it = std::find_if(range.begin(), range.end(), [=](const auto& socket_ptr) { return SDL_PointInRect(&space_point, &(socket_ptr->GetSpaceRect())); });
            if (it != range.end())
            {
                return *it;
            }
        }
    }
    return nullptr;
}

bool node::GraphicsScene::InternalSelectObject(GraphicsObject* object)
{
    if (object->isSelectable())
    {
        if (!IsObjectSelected(*object))
        {
            ClearCurrentSelection();
            AddSelection(object->GetFocusHandlePtr());
        }
        return true;
    }
    else
    {
        ClearCurrentSelection();
        return false;
    }
}

void node::GraphicsScene::OnMouseMove(const SDL_Point& p)
{
    switch (m_mouse_capture_mode)
    {
    case node::GraphicsScene::CAPTURE_MODE::NONE:
        {
            node::GraphicsObject* current_hover = GetInteractableAt(p);
            SetCurrentHover(current_hover);
            if (current_hover)
            {
                auto&& transformer = GetSpaceScreenTransformer();
                current_hover->MouseMove(transformer.ScreenToSpacePoint(p));
            }
            break;
        }
    case node::GraphicsScene::CAPTURE_MODE::OBJECT:
    {
        node::GraphicsObject* current_hover = m_current_mouse_hover.GetObjectPtr();
        SDL_assert(current_hover);
        auto&& transformer = GetSpaceScreenTransformer();
        current_hover->MouseMove(transformer.ScreenToSpacePoint(p));
        break;
    }
    case node::GraphicsScene::CAPTURE_MODE::OBJECTS_DRAG:
    {
        SDL_assert(m_drag_objects.size());
        auto&& transformer = GetSpaceScreenTransformer();
        const SDL_Point drag_vector = transformer.ScreenToSpaceVector({p.x - m_StartPointScreen.x, p.y - m_StartPointScreen.y});
        for (auto&& drag_object : m_drag_objects)
        {
            node::GraphicsObject* object =  drag_object.m_object.GetObjectPtr();
            if (object)
            {
                object->SetSpaceOrigin(QuantizePoint({
                    drag_object.m_start_position.x + drag_vector.x,
                    drag_object.m_start_position.y + drag_vector.y
                    }));
            }
        }
        UpdateObjectsRect();
        InvalidateRect();
        break;
    }
    case node::GraphicsScene::CAPTURE_MODE::SCREEN_DRAG:
    {
        auto&& transformer = GetSpaceScreenTransformer();
        const SDL_Point current_position_difference_space_vector = transformer.ScreenToSpaceVector({
        p.x - m_StartPointScreen.x, p.y - m_StartPointScreen.y
            });

        SetSpaceRect({
            m_startEdgeSpace.x - current_position_difference_space_vector.x,
            m_startEdgeSpace.y - current_position_difference_space_vector.y,
            GetSpaceRect().w,
            GetSpaceRect().h
            });
        UpdateObjectsRect();
        InvalidateRect();
        break;
    }
    }
}

MI::ClickEvent node::GraphicsScene::OnLMBDown(const SDL_Point& p)
{
    node::GraphicsObject* current_hover = m_current_mouse_hover.GetObjectPtr();
    if (current_hover)
    {
        // selection code
        InternalSelectObject(current_hover);
        
        // do Click
        auto&& transformer = GetSpaceScreenTransformer();
        SDL_Point SpacePoint = transformer.ScreenToSpacePoint(p);
        MI::ClickEvent result = current_hover->LMBDown(SpacePoint);
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
                m_drag_objects.emplace_back(object->GetMIHandlePtr(), SDL_Point{ object->GetSpaceRect().x, object->GetSpaceRect().y});
            }
        }
        if (m_drag_objects.size() != 0)
        {
            m_mouse_capture_mode = node::GraphicsScene::CAPTURE_MODE::OBJECTS_DRAG;
            m_StartPointScreen = p;
            return MI::ClickEvent::CAPTURE_START;
        }
    }
    else
    {
        ClearCurrentSelection();
        m_mouse_capture_mode = node::GraphicsScene::CAPTURE_MODE::SCREEN_DRAG;
        m_StartPointScreen = p;
        m_startEdgeSpace = { m_spaceRect.x, m_spaceRect.y };
        return MI::ClickEvent::CAPTURE_START;
    }
    return MI::ClickEvent::NONE;

}

MI::ClickEvent node::GraphicsScene::OnLMBUp(const SDL_Point& p)
{
    switch (m_mouse_capture_mode)
    {
    case node::GraphicsScene::CAPTURE_MODE::NONE:
    {
        return MI::ClickEvent::NONE;
        break;
    }
    case node::GraphicsScene::CAPTURE_MODE::OBJECT:
    {
        node::GraphicsObject* current_hover = m_current_mouse_hover.GetObjectPtr();
        if (current_hover)
        {
            auto&& transformer = GetSpaceScreenTransformer();
            SDL_Point SpacePoint = transformer.ScreenToSpacePoint(p);
            current_hover->LMBUp(SpacePoint);
            m_mouse_capture_mode = node::GraphicsScene::CAPTURE_MODE::NONE;
            return MI::ClickEvent::CAPTURE_END;
        }
        break;
    }
    case node::GraphicsScene::CAPTURE_MODE::OBJECTS_DRAG:
    {
        m_drag_objects.clear();
        m_mouse_capture_mode = node::GraphicsScene::CAPTURE_MODE::NONE;
        return MI::ClickEvent::CAPTURE_END;
        break;
    }
    case node::GraphicsScene::CAPTURE_MODE::SCREEN_DRAG:
    {
        m_mouse_capture_mode = node::GraphicsScene::CAPTURE_MODE::NONE;
        return MI::ClickEvent::CAPTURE_END;
        break;
    }
    }
    return MI::ClickEvent::NONE;
}

void node::GraphicsScene::SetSpaceRect(const SDL_Rect& rect)
{
    m_spaceRect = rect;
    m_spaceScreenTransformer = SpaceScreenTransformer{ GetRect(), m_spaceRect };
}
const SDL_Rect& node::GraphicsScene::GetSpaceRect() const noexcept
{
    return m_spaceRect;
}

const SDL_Rect& node::GraphicsScene::GetSpaceRectBase() const noexcept
{
    return m_spaceRect_base;
}

std::span<const node::HandlePtr<node::GraphicsObject>> node::GraphicsScene::GetCurrentSelection() const
{
    return this->m_current_selection;
}
std::span<node::HandlePtr<node::GraphicsObject>> node::GraphicsScene::GetCurrentSelection()
{
    return this->m_current_selection;
}

void node::GraphicsScene::AddSelection(HandlePtr<GraphicsObject> handle)
{
    this->m_current_selection.push_back(handle);
}

bool node::GraphicsScene::IsObjectSelected(const GraphicsObject& obj) const
{
    const node::GraphicsObject* obj_ptr = &obj;
    auto iter = std::find_if(m_current_selection.begin(), m_current_selection.end(),
        [obj_ptr](const node::HandlePtr<node::GraphicsObject>& c_ptr) ->bool {
            if (c_ptr.isAlive())
            {
                if (c_ptr.GetObjectPtr() == obj_ptr)
                {
                    return true;
                }
            }
            return false;
            });
    if (iter != m_current_selection.end())
    {
        return true;
    }
    return false;
}

void node::GraphicsScene::ClearCurrentSelection()
{
    this->m_current_selection.clear();
}


bool node::GraphicsScene::OnScroll(const double amount, const SDL_Point& p)
{

    // 1. save old pointer position
// 2. grow width and height
// 3. get new screen point position
// 4. change origin by the difference between them
    SDL_Point old_position;
    if (amount > 0)
    {
        if (m_zoomScale <= 0.3)
        {
            return false;
        }
        auto&& transformer = GetSpaceScreenTransformer();
        old_position = transformer.ScreenToSpacePoint(p);
        m_zoomScale /= m_scroll_ratio;
        if (m_zoomScale < 0.3)
        {
            m_zoomScale = 0.3;
        }

    }
    else if (amount < 0)
    {
        if (m_zoomScale >= 2)
        {
            return false;
        }
        auto&& transformer = GetSpaceScreenTransformer();
        old_position = transformer.ScreenToSpacePoint(p);
        m_zoomScale *= m_scroll_ratio;
        if (m_zoomScale > 2)
        {
            m_zoomScale = 2;
        }
    }
    else {
        return false;
    }
    SDL_Rect new_rect = GetSpaceRect();
    new_rect.w = static_cast<int>(GetBaseRect().w * m_zoomScale);
    new_rect.h = static_cast<int>(GetBaseRect().h * m_zoomScale);
    SetSpaceRect(new_rect);
    auto&& transformer = GetSpaceScreenTransformer();
    SDL_Point new_position = transformer.ScreenToSpacePoint(p);
    new_rect = GetSpaceRect();
    new_rect.x = new_rect.x + old_position.x - new_position.x;
    new_rect.y = new_rect.y + old_position.y - new_position.y;
    SetSpaceRect(new_rect);
    UpdateObjectsRect();
    InvalidateRect();
    return true;
}

void node::GraphicsScene::UpdateObjectsRect()
{
    for (auto& obj: m_objects)
    {
        obj.m_ptr->UpdateRect();
    }
}

void node::GraphicsScene::InvalidateRect()
{
    Widget::InvalidateRect();
}

void node::GraphicsScene::SetGraphicsLogic(std::unique_ptr<GraphicsLogic> logic)
{
    if (m_graphicsLogic)
    {
        m_graphicsLogic->Cancel();
    }
    m_graphicsLogic = std::move(logic);
}

node::IGraphicsSceneController* node::GraphicsScene::GetController() const
{
    return nullptr;
}

void node::GraphicsScene::OnSetRect(const SDL_Rect& rect)
{
    double x_ratio = static_cast<double>(rect.w)/m_rect_base.w;
    double y_ratio = static_cast<double>(rect.h)/m_rect_base.h;
    m_spaceRect.w = static_cast<int>(m_spaceRect_base.w * x_ratio);
    m_spaceRect.h = static_cast<int>(m_spaceRect_base.h * y_ratio);
    m_spaceScreenTransformer = SpaceScreenTransformer{ GetRect(), m_spaceRect };
    Widget::OnSetRect(rect);
}

node::GraphicsObject* node::GraphicsScene::GetInteractableAt(const SDL_Point& p) const
{
    for (auto& object: m_objects)
    {
        node::GraphicsObject* current_hover = object.m_ptr->GetInteractableAtPoint(p);
        if (current_hover != nullptr)
        {
            return current_hover;
        }
    }
    return nullptr;
}

const node::SpaceScreenTransformer& node::GraphicsScene::GetSpaceScreenTransformer() const
{
    return m_spaceScreenTransformer;
}

SDL_Point node::GraphicsScene::QuantizePoint(const SDL_Point& p)
{
    return { 
        static_cast<int>(p.x/m_spaceQuantization)*m_spaceQuantization,
        static_cast<int>(p.y/m_spaceQuantization)*m_spaceQuantization
        };
}

std::vector<node::Node*> node::GraphicsScene::GetNodes()
{
    std::vector<node::Node*> out;
    for (auto& pointer : m_objects)
    {
        if (ObjectType::node == pointer.m_ptr->GetObjectType())
        {
            auto ptr = static_cast<node::Node*>(pointer.m_ptr.get());
            out.push_back(ptr);
        }
    }
    return out;
}
