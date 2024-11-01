#include "GraphicsScene.hpp"
#include "toolgui/NodeMacros.h"
#include <algorithm>
#include "GraphicsScene/GraphicsObject.hpp"
#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/ContextMenu.hpp"
#include "NodeSDLStylers/BlockStyler.hpp"
#include "NodeModels/SceneModelManager.hpp"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"

node::GraphicsScene::GraphicsScene(const SDL_FRect& rect, node::Scene* parent)
:Widget(rect, parent), 
m_spaceRect_base{0, 0, 1000, static_cast<int>(1000 * rect.h/rect.w)}, 
m_spaceRect{0, 0, 200, static_cast<int>(200 * rect.h/rect.w)},
m_spaceScreenTransformer(GetRect(), m_spaceRect)
{
    SetDropTarget(true);
}

node::GraphicsScene::~GraphicsScene()
{

}

void node::GraphicsScene::AddObject(std::unique_ptr<node::GraphicsObject> obj, int z_order)
{
    GraphicsObject* object = obj.get();
    ObjectSlot slot = {std::move(obj), z_order};
    auto iter = std::lower_bound(m_objects.begin(), m_objects.end(), slot, [](const auto& obj1, const auto& obj2) {return obj1.z_order > obj2.z_order;} );
    m_objects.insert(iter, std::move(slot));
    object->SetScene(this);
    object->SetSelected(false);
}



std::unique_ptr<node::GraphicsObject> node::GraphicsScene::PopObject(const node::GraphicsObject* obj)
{
    auto iter = std::find_if(m_objects.begin(), m_objects.end(), [=](const auto& item) {return item.m_ptr.get() == obj;});
    if (iter != m_objects.end())
    {
        std::unique_ptr<node::GraphicsObject> ret_obj = std::move(iter->m_ptr);
        m_objects.erase(iter);
        return ret_obj;
    }
    return nullptr;
}

void node::GraphicsScene::ClearAllObjects()
{
    m_objects.clear();
}

void node::GraphicsScene::BumpObjectInLayer(node::GraphicsObject* obj)
{
    if (m_objects.size() < 2)
    {
        return;
    }

    auto it = std::find_if(m_objects.begin(), m_objects.end(), [&](const ObjectSlot& slot) { return slot.m_ptr.get() == obj; });
    if (it != m_objects.end())
    {
        auto iter2 = std::lower_bound(m_objects.begin(), m_objects.end(), *it, [](const auto& obj1, const auto& obj2) {return obj1.z_order > obj2.z_order; });
        if (iter2 != m_objects.end())
        {
            std::swap(*it, *iter2);
        }
    }
}

void node::GraphicsScene::Draw(SDL_Renderer *renderer)
{
    SDL_FRect screen_rect = ToSDLRect(GetSpaceRect());
    for (auto&& it = m_objects.rbegin(); it != m_objects.rend(); it++)
    {
        auto&& object = *it;
        SDL_FRect obj_rect = ToSDLRect(object.m_ptr->GetSpaceRect());
        if (object.m_ptr->IsVisible() && SDL_HasRectIntersectionFloat(&screen_rect, &obj_rect))
        {
            object.m_ptr->Draw(renderer, GetSpaceScreenTransformer());
        }
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

node::BlockSocketObject* node::GraphicsScene::GetSocketAt(const model::Point space_point)
{
    for (auto& object : m_objects)
    {
        SDL_FRect object_space_rect = ToFRect({ object.m_ptr->GetSpaceRect().x, object.m_ptr->GetSpaceRect().y,
            object.m_ptr->GetSpaceRect().w, object.m_ptr->GetSpaceRect().h });
        SDL_FPoint space_point_sdl = ToSDLPoint(space_point);
        if (ObjectType::block == object.m_ptr->GetObjectType() && SDL_PointInRectFloat(&space_point_sdl, &object_space_rect))
        {
            
            node::BlockObject* node_pointer = static_cast<node::BlockObject*>(object.m_ptr.get());
            auto&& range = node_pointer->GetSockets();
            auto it = std::find_if(range.begin(), range.end(), 
                [=](const auto& socket_ptr) { 
                SDL_FRect socket_rect_sdl = ToSDLRect(socket_ptr->GetSpaceRect());
                return SDL_PointInRectFloat(&space_point_sdl, &socket_rect_sdl); 
                });
            if (it != range.end())
            {
                return it->get();
            }
        }
    }
    return nullptr;
}

void node::GraphicsScene::OnDropObject(DragDropObject& object, const SDL_FPoint& p)
{
    Notify(BlockObjectDropped{ object, p });
    m_dragDropDrawObject = std::nullopt;
}

void node::GraphicsScene::OnDrawDropObject(SDL_Renderer* renderer, const DragDropObject& object, const SDL_FPoint& p)
{
    UNUSED_PARAM(object);
    model::Rect bounds = m_dragDropDrawObject->model.GetBounds();
    model::Point offset = { -bounds.w / 2, -bounds.h / 2 };
    auto point = QuantizePoint(m_spaceScreenTransformer.ScreenToSpacePoint(p) + offset);
    m_dragDropDrawObject->model.SetPosition(point);
    bounds = m_dragDropDrawObject->model.GetBounds();
    m_dragDropDrawObject->styler->DrawBlockOutline(renderer, bounds, 
        m_spaceScreenTransformer, m_dragDropDrawObject->model.GetOrienation(), true);
    for (const auto& socket : m_dragDropDrawObject->model.GetSockets())
    {
        m_dragDropDrawObject->styler->DrawBlockSocket(renderer, socket.GetPosition() + model::Point{point}, 
            m_spaceScreenTransformer, socket.GetType());
    }
    m_dragDropDrawObject->styler->DrawBlockDetails(renderer, bounds, m_spaceScreenTransformer, m_dragDropDrawObject->model.GetOrienation(), true);
}

void node::GraphicsScene::OnDropEnter(const DragDropObject& object)
{
    m_dragDropDrawObject = DragDropDrawObject{ object.block, object.styler };
}

void node::GraphicsScene::OnDropExit(const DragDropObject& object)
{
    UNUSED_PARAM(object);
    m_dragDropDrawObject = std::nullopt;
}


void node::GraphicsScene::OnMouseMove(MouseHoverEvent& e)
{
    SDL_FPoint p{ e.point() };
    m_current_mouse_position = p;
    model::Point point = m_spaceScreenTransformer.ScreenToSpacePoint(p);
    if (m_graphicsLogic)
    {
        m_graphicsLogic->MouseMove(point);
        return;
    }

    if (!m_tool || !m_tool->IsCapturingMouse())
    {
        node::GraphicsObject* current_hover = GetObjectAt(point);
        SetCurrentHover(current_hover);
    }
    
    if (m_tool)
    {
        GraphicsTool::MouseHoverEvent ev{ point };
        m_tool->OnMouseMove(ev);
    }
}

MI::ClickEvent node::GraphicsScene::OnLMBDown(MouseButtonEvent& e)
{
    SDL_FPoint p{ e.point() };
    model::Point point = m_spaceScreenTransformer.ScreenToSpacePoint(p);

    assert(m_graphicsLogic == nullptr);
    if (m_tool)
    {
        GraphicsTool::MouseButtonEvent ev{ point, e.e.clicks == 2 };
        return m_tool->OnLMBDown(ev);
    }
    return MI::ClickEvent::NONE;

}

MI::ClickEvent node::GraphicsScene::OnLMBUp(MouseButtonEvent& e)
{
    SDL_FPoint p{ e.point() };
    auto&& transformer = GetSpaceScreenTransformer();
    model::Point SpacePoint = transformer.ScreenToSpacePoint(p);
    if (m_graphicsLogic)
    {
        auto result = m_graphicsLogic->LMBUp(SpacePoint);
        m_graphicsLogic->SetDone(true);
        SetGraphicsLogic(nullptr);
        return result;
    }

    if (m_tool)
    {
        GraphicsTool::MouseButtonEvent ev{ SpacePoint, e.e.clicks == 2};
        return m_tool->OnLMBUp(ev);
    }
    return MI::ClickEvent::NONE;
}

void node::GraphicsScene::SetSpaceRect(const model::Rect& rect)
{
    m_spaceRect = rect;
    m_spaceScreenTransformer = SpaceScreenTransformer{ GetRect(), m_spaceRect };
}
const node::model::Rect& node::GraphicsScene::GetSpaceRect() const noexcept
{
    return m_spaceRect;
}

std::span<const node::HandlePtr<node::GraphicsObject>> node::GraphicsScene::GetCurrentSelection() const
{
    return m_current_selection;
}
std::span<node::HandlePtr<node::GraphicsObject>> node::GraphicsScene::GetCurrentSelection()
{
    return m_current_selection;
}

void node::GraphicsScene::AddSelection(HandlePtr<GraphicsObject> handle)
{
    if (handle->IsSelected())
    {
        return;
    }
    handle->SetSelected(true);
    this->m_current_selection.push_back(handle);
}

bool node::GraphicsScene::IsObjectSelected(const GraphicsObject& obj) const
{
    const node::GraphicsObject* obj_ptr = &obj;
    auto iter = std::find_if(m_current_selection.begin(), m_current_selection.end(),
        [obj_ptr](const node::HandlePtr<node::GraphicsObject>& c_ptr) ->bool {
            if (c_ptr.GetObjectPtr() == obj_ptr)
            {
                return true;
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
    for (auto&& item : m_current_selection)
    {
        if (item)
        {
            item->SetSelected(false);
        }
    }
    this->m_current_selection.clear();
}


bool node::GraphicsScene::OnScroll(const double amount, const SDL_FPoint& p)
{

    // 1. save old pointer position
    // 2. grow width and height
    // 3. get new screen point position
    // 4. change origin by the difference between them
    model::Point old_position;
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
    model::Rect new_rect = GetSpaceRect();
    SDL_FRect rect_base = GetBaseRect();
    double x_ratio = static_cast<double>(GetRect().w) / rect_base.w;
    double y_ratio = static_cast<double>(GetRect().h) / rect_base.h;
    new_rect.w = static_cast<int>(m_spaceRect_base.w * x_ratio * m_zoomScale);
    new_rect.h = static_cast<int>(m_spaceRect_base.h * y_ratio * m_zoomScale);
    SetSpaceRect(new_rect);
    auto&& transformer = GetSpaceScreenTransformer();
    model::Point new_position = transformer.ScreenToSpacePoint(p);
    new_rect = GetSpaceRect();
    new_rect.x = new_rect.x + old_position.x - new_position.x;
    new_rect.y = new_rect.y + old_position.y - new_position.y;
    SetSpaceRect(new_rect);
    UpdateObjectsRect();
    return true;
}

void node::GraphicsScene::UpdateObjectsRect()
{
    for (auto& obj: m_objects)
    {
        obj.m_ptr->UpdateRect();
    }
}

void node::GraphicsScene::SetGraphicsLogic(std::unique_ptr<logic::GraphicsLogic> logic)
{
    if (m_graphicsLogic && !m_graphicsLogic->IsDone())
    {
        m_graphicsLogic->Cancel();
    }
    m_graphicsLogic = std::move(logic);
    if (m_graphicsLogic)
    {
        m_graphicsLogic->Start(m_spaceScreenTransformer.ScreenToSpacePoint(m_current_mouse_position));
    }
}

void  node::GraphicsScene::CancelCurrentLogic()
{
    SetGraphicsLogic(nullptr);
}


void node::GraphicsScene::SetTool(std::shared_ptr<GraphicsTool> ptr)
{
    if (m_tool)
    {
        m_tool->OnExit();
    }
    m_tool = std::move(ptr);
    m_tool->OnStart();

    GraphicsTool::MouseHoverEvent e{ m_spaceScreenTransformer.ScreenToSpacePoint(m_current_mouse_position) };
    m_tool->OnMouseEnter(e);
    
}

void node::GraphicsScene::OnSetRect(const SDL_FRect& rect)
{
    Widget::OnSetRect(rect);
    SDL_FRect rect_base = GetBaseRect();
    double x_ratio = static_cast<double>(rect.w)/ rect_base.w;
    double y_ratio = static_cast<double>(rect.h)/ rect_base.h;
    m_spaceRect.w = static_cast<int>(m_spaceRect_base.w * x_ratio * m_zoomScale);
    m_spaceRect.h = static_cast<int>(m_spaceRect_base.h * y_ratio * m_zoomScale);
    m_spaceScreenTransformer = SpaceScreenTransformer{ GetRect(), m_spaceRect };
}

node::GraphicsObject* node::GraphicsScene::GetObjectAt(const model::Point& p) const
{
    for (auto& object: m_objects)
    {
        node::GraphicsObject* current_hover = object.m_ptr->GetInteractableAtPoint(p);
        if (current_hover)
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

node::model::Point node::GraphicsScene::QuantizePoint(const model::Point& p)
{
    return { 
        static_cast<int>(p.x/m_spaceQuantization)*m_spaceQuantization,
        static_cast<int>(p.y/m_spaceQuantization)*m_spaceQuantization
        };
}
