#include "GraphicsScene.hpp"
#include "toolgui/NodeMacros.h"
#include "GraphicsScene/BlockObject.hpp"
#include "GraphicsScene/BlockSocketObject.hpp"
#include <algorithm>
#include "GraphicsScene/GraphicsObject.hpp"
#include "GraphicsScene/GraphicsLogic/GraphicsLogic.hpp"
#include "toolgui/Scene.hpp"
#include "toolgui/ContextMenu.hpp"
#include "NodeSDLStylers/BlockStyler.hpp"
#include "NodeModels/SceneModelManager.hpp"

node::GraphicsScene::GraphicsScene(const SDL_Rect& rect, node::Scene* parent)
:Widget(rect, parent), 
m_spaceRect_base{0, 0, 1000, 1000 * rect.h/rect.w}, 
m_spaceRect{0, 0, 200, 200 * rect.h/rect.w},
m_spaceScreenTransformer(GetRect(), m_spaceRect)
{
    SetDropTarget(true);
}

node::GraphicsScene::~GraphicsScene()
{

}

void node::GraphicsScene::AddObject(std::unique_ptr<node::GraphicsObject> obj, int z_order)
{
    obj->SetSpaceOrigin(
        obj->isAligned() ? QuantizePoint({obj->GetSpaceRect().x, obj->GetSpaceRect().y}):
    model::Point{obj->GetSpaceRect().x, obj->GetSpaceRect().y}
    );
    ObjectSlot slot = {std::move(obj), z_order};
    auto iter = std::lower_bound(m_objects.begin(), m_objects.end(), slot, [](const auto& obj1, const auto& obj2) {return obj1.z_order > obj2.z_order;} );
    m_objects.insert(iter, std::move(slot));
}

void node::GraphicsScene::SetSceneModel(std::shared_ptr<SceneModelManager> scene)
{
    m_objects.clear();
    m_drag_objects.clear();
    m_graphicsLogic = nullptr;
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
        std::unique_ptr<node::BlockObject> obj = std::make_unique<node::BlockObject>(this, block, styler);
        AddObject(std::move(obj), 0);
    }
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

node::BlockSocketObject* node::GraphicsScene::GetSocketAt(const model::Point space_point)
{
    for (auto& object : m_objects)
    {
        SDL_Rect object_space_rect = { object.m_ptr->GetSpaceRect().x, object.m_ptr->GetSpaceRect().y,
            object.m_ptr->GetSpaceRect().w, object.m_ptr->GetSpaceRect().h };
        SDL_Point space_point_sdl = ToSDLPoint(space_point);
        if (ObjectType::block == object.m_ptr->GetObjectType() && SDL_PointInRect(&space_point_sdl, &object_space_rect))
        {
            
            node::BlockObject* node_pointer = static_cast<node::BlockObject*>(object.m_ptr.get());
            auto&& range = node_pointer->GetSockets();
            auto it = std::find_if(range.begin(), range.end(), 
                [=](const auto& socket_ptr) { 
                SDL_Rect socket_rect_sdl = ToSDLRect(socket_ptr->GetSpaceRect());
                return SDL_PointInRect(&space_point_sdl, &socket_rect_sdl); 
                });
            if (it != range.end())
            {
                return *it;
            }
        }
    }
    return nullptr;
}

void node::GraphicsScene::OnDropObject(DragDropObject& object, const SDL_Point& p)
{
    auto block = std::make_shared<model::BlockModel>(model::BlockModel{ object.block });

    model::Rect bounds = m_dragDropDrawObject->model.GetBounds();
    model::Point offset = { -bounds.w / 2, -bounds.h / 2 };
    block->SetPosition(QuantizePoint(m_spaceScreenTransformer.ScreenToSpacePoint(p) + offset));

    auto styler = std::make_shared<node::BlockStyler>();
    styler->PositionNodes(*block);
    m_sceneModel->AddBlock(block);
    m_dragDropDrawObject = std::nullopt;
}

void node::GraphicsScene::OnDrawDropObject(SDL_Renderer* renderer, const DragDropObject& object, const SDL_Point& p)
{
    UNUSED_PARAM(object);
    model::Rect bounds = m_dragDropDrawObject->model.GetBounds();
    model::Point offset = { -bounds.w / 2, -bounds.h / 2 };
    auto point = QuantizePoint(m_spaceScreenTransformer.ScreenToSpacePoint(p) + offset);
    m_dragDropDrawObject->model.SetPosition(point);
    m_dragDropDrawObject->styler.DrawBlock(renderer, m_dragDropDrawObject->model, m_spaceScreenTransformer, false);
}

void node::GraphicsScene::OnDropEnter(const DragDropObject& object)
{
    m_dragDropDrawObject = DragDropDrawObject{ object.block, BlockStyler{} };
}

void node::GraphicsScene::OnDropExit(const DragDropObject& object)
{
    UNUSED_PARAM(object);
    m_dragDropDrawObject = std::nullopt;
}

void node::GraphicsScene::OnNotify(SceneModification& e)
{
    switch (e.type)
    {
    case SceneModification::type_t::BlockAdded:
    {
        auto styler = std::make_shared<node::BlockStyler>();
        std::unique_ptr<node::BlockObject> obj = std::make_unique<node::BlockObject>(this, std::get<model::BlockModelPtr>(e.data), styler);
        auto* ptr = obj.get();
        AddObject(std::move(obj), 0);
        ClearCurrentSelection();
        AddSelection(ptr->GetFocusHandlePtr());
        break;
    }
    case SceneModificationType::BlockRemoved:
    {
        auto model_id = std::get<model::BlockModelPtr>(e.data)->GetId();
        auto it = std::find_if(m_objects.begin(), m_objects.end(), [&](auto&& object)
            {
                if (object.m_ptr->GetObjectType() == ObjectType::block)
                {
                    if (static_cast<BlockObject*>(object.m_ptr.get())->GetModelId() == model_id)
                    {
                        return true;
                    }
                }
                return false;
            });
        if (it != m_objects.end())
        {
            m_objects.erase(it);
        }
        break;
    }
    
    }
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
        m_tool->OnMouseMove(point);
    }
    /*
    switch (m_mouse_capture_mode)
    {
    case node::GraphicsScene::CAPTURE_MODE::NONE:
        {
            
        }
    case node::GraphicsScene::CAPTURE_MODE::OBJECT:
    {
        node::GraphicsObject* current_hover = m_current_mouse_hover.GetObjectPtr();
        SDL_assert(current_hover);
        current_hover->MouseMove(point);
        break;
    }
    case node::GraphicsScene::CAPTURE_MODE::OBJECTS_DRAG:
    {
        SDL_assert(m_drag_objects.size());
        auto&& transformer = GetSpaceScreenTransformer();
        const auto drag_vector = transformer.ScreenToSpaceVector({p.x - m_StartPointScreen.x, p.y - m_StartPointScreen.y});
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
        const auto current_position_difference_space_vector = transformer.ScreenToSpaceVector({
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
    */
}

MI::ClickEvent node::GraphicsScene::OnLMBDown(const SDL_Point& p)
{
    model::Point point = m_spaceScreenTransformer.ScreenToSpacePoint(p);

    assert(m_graphicsLogic == nullptr);
    if (m_tool)
    {
        return m_tool->OnLMBDown(point);
    }
    return MI::ClickEvent::NONE;

}

MI::ClickEvent node::GraphicsScene::OnLMBUp(const SDL_Point& p)
{
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
        return m_tool->OnLMBUp(SpacePoint);
    }
    /*
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
            current_hover->LMBUp({ SpacePoint.x, SpacePoint.y});
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
    */
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

const node::model::Rect& node::GraphicsScene::GetSpaceRectBase() const noexcept
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
    double x_ratio = static_cast<double>(GetRect().w) / m_rect_base.w;
    double y_ratio = static_cast<double>(GetRect().h) / m_rect_base.h;
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

void node::GraphicsScene::SetGraphicsLogic(std::unique_ptr<logic::GraphicsLogic> logic)
{
    if (m_graphicsLogic && !m_graphicsLogic->IsDone())
    {
        m_graphicsLogic->Cancel();
    }
    m_graphicsLogic = std::move(logic);
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
    if (m_current_mouse_position)
    {
        m_tool->OnMouseEnter(m_spaceScreenTransformer.ScreenToSpacePoint(*m_current_mouse_position));
    }
}

void node::GraphicsScene::OnSetRect(const SDL_Rect& rect)
{
    Widget::OnSetRect(rect);
    double x_ratio = static_cast<double>(rect.w)/m_rect_base.w;
    double y_ratio = static_cast<double>(rect.h)/m_rect_base.h;
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

std::vector<node::BlockObject*> node::GraphicsScene::GetNodes()
{
    std::vector<node::BlockObject*> out;
    for (auto& pointer : m_objects)
    {
        if (ObjectType::block == pointer.m_ptr->GetObjectType())
        {
            auto ptr = static_cast<node::BlockObject*>(pointer.m_ptr.get());
            out.push_back(ptr);
        }
    }
    return out;
}
