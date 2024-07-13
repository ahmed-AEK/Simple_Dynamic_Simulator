

#include "Scene.hpp"
#include "algorithm"
#include "SDL_Framework/SDL_headers.h"
#include "toolgui/ContextMenu.hpp"
#include "toolgui/Application.hpp"
#include "toolgui/Widget.hpp"
#include "toolgui/SidePanel.hpp"

void node::Scene::Draw(SDL_Renderer* renderer)
{
    OnDraw(renderer);
    if (m_current_mouse_hover.isAlive() && m_dragObject)
    {
        SDL_Point p;
        SDL_GetMouseState(&p.x, &p.y);
        m_current_mouse_hover.GetObjectPtr()->DrawDropObject(renderer, *m_dragObject, p);
    }
}

void node::Scene::OnDraw(SDL_Renderer* renderer)
{
    for (auto&& it = m_widgets.rbegin(); it != m_widgets.rend(); it++)
    {
        auto&& widget = *it;
        widget.m_ptr->Draw(renderer);
    }
    if (m_sidePanel)
    {
        m_sidePanel->Draw(renderer);
    }
    if (m_pContextMenu != nullptr)
    {
        m_pContextMenu->Draw(renderer);
    }
}

void node::Scene::ShowContextMenu(std::unique_ptr<node::ContextMenu> menu, const SDL_Point& p)
{
    m_pContextMenu = std::move(menu);
    m_pContextMenu->SetRect({p.x,p.y, 0,0});

}
void node::Scene::DestroyContextMenu()
{
    m_pContextMenu.reset(nullptr);
}

node::SceneWidgetIterator node::Scene::begin()
{
    return SceneWidgetIterator(this, 0);
}

node::SceneWidgetIterator node::Scene::end()
{
    return SceneWidgetIterator(this, this->m_widgets.size());
}

void node::Scene::SetSidePanel(std::unique_ptr<SidePanel> panel)
{
    m_sidePanel = std::move(panel); 
}

void node::Scene::DoUpdateTasks()
{
    for (auto&& [key, value] : m_new_updateTasks)
    {
        m_updateTasks.emplace(key, std::move(value));
    }
    m_new_updateTasks.clear();

    for (auto&& id : m_deleted_updateTasks)
    {
        auto it = m_updateTasks.find(id);
        assert(it != m_updateTasks.end());
        if (it != m_updateTasks.end())
        {
            m_updateTasks.erase(it);
        }
    }
    m_deleted_updateTasks.clear();
    
    for (auto&& task : m_updateTasks)
    {
        if (task.second.widget.isAlive())
        {
            task.second.task();
        }
        else
        {
            m_deleted_updateTasks.push_back(task.first);
        }
    }
}

int64_t node::Scene::AddUpdateTask(UpdateTask task)
{
    auto current_task = m_current_task_id;
    m_new_updateTasks.emplace(current_task, std::move(task));
    m_current_task_id++;
    return current_task;
}

void node::Scene::RemoveUpdateTask(int64_t task_id)
{
    m_deleted_updateTasks.push_back(task_id);
}

void node::Scene::StartDragObject(DragDropObject object)
{
    m_dragObject = std::move(object);
    if (m_current_mouse_hover.isAlive() && m_current_mouse_hover.GetObjectPtr()->IsDropTarget())
    {
        m_current_mouse_hover.GetObjectPtr()->DropEnter(*m_dragObject);
        SDL_Point p;
        SDL_GetMouseState(&p.x, &p.y);
        m_current_mouse_hover.GetObjectPtr()->DropHover(*m_dragObject, p);
    }
    
}

void node::Scene::OnMouseMove(const SDL_Point& p)
{
    if (!b_mouseCaptured)
    {
        node::Widget* current_hover = this->GetInteractableAt(p);
        node::Widget* old_hover = nullptr;
        if (m_current_mouse_hover.isAlive())
        {
            old_hover = m_current_mouse_hover.GetObjectPtr();
        }
        if (current_hover != old_hover)
        {
            if (old_hover)
            {
                old_hover->MouseOut();
                if (m_dragObject && old_hover->IsDropTarget())
                {
                    current_hover->DropExit(*m_dragObject);
                }
            }

            if (current_hover)
            {
                m_current_mouse_hover = current_hover->GetMIHandlePtr();
            }
            else
            {
                m_current_mouse_hover = node::HandlePtr<node::Widget>{};
            }

            if (current_hover)
            {
                current_hover->MouseIn();
                if (m_dragObject && current_hover->IsDropTarget())
                {
                    current_hover->DropEnter(*m_dragObject);
                }
            }
        }
        
    }
    if (m_current_mouse_hover.isAlive())
    {
        if (auto object = m_current_mouse_hover.GetObjectPtr())
        {
            if (object->IsDropTarget() && m_dragObject)
            {
                object->DropHover(*m_dragObject, p);
            }
            else
            {
                object->MouseMove({ p.x, p.y });
            }
        }
    }
}

node::Widget* node::Scene::GetInteractableAt(const SDL_Point& p) const
{
    if (m_pContextMenu != nullptr)
    {
        if (auto interactable = m_pContextMenu->GetInteractableAtPoint(p))
        {
            return interactable;
        }
    }
    if (m_sidePanel)
    {
        if (auto result = m_sidePanel->GetInteractableAtPoint(p))
        {
            return result;
        }
    }
    for (auto& widget: m_widgets)
    {
        node::Widget* current_hover = widget.m_ptr->GetInteractableAtPoint(p);
        if (current_hover != nullptr)
        {
            return current_hover;
        }
    }
    return nullptr;
}

void node::Scene::AddWidget(std::unique_ptr<Widget> widget, int z_order)
{
    WidgetSlot slot = {std::move(widget), z_order};
    auto iter = std::lower_bound(m_widgets.begin(), m_widgets.end(), slot, [](const auto& obj1, const auto& obj2) {return obj1.z_order > obj2.z_order;} );
    m_widgets.insert(iter, std::move(slot));
}

void node::Scene::SetRect(const SDL_Rect& rect)
{
    OnSetRect(rect);
}

void node::Scene::Start()
{
    OnStart();
}

void node::Scene::OnSetRect(const SDL_Rect& rect)
{
    m_sidePanel->UpdateWindowSize(rect);

    double x_ratio = static_cast<double>(rect.w)/m_rect_base.w;
    double y_ratio = static_cast<double>(rect.h)/m_rect_base.h;
    m_rect = rect;
    for (auto& widget : m_widgets)
    {
        if (WidgetScaling::FixedPixels == widget.m_ptr->GetScalingType())
        {
            continue;
        }
        SDL_Rect modified_rect = widget.m_ptr->GetBaseRect();
        modified_rect.x = static_cast<int>(modified_rect.x * x_ratio);
        modified_rect.w = static_cast<int>(modified_rect.w * x_ratio);
        modified_rect.y = static_cast<int>(modified_rect.y * y_ratio);
        modified_rect.h = static_cast<int>(modified_rect.h * y_ratio);
        widget.m_ptr->SetRect(modified_rect);
    }
}

bool node::Scene::OnLMBDown(const SDL_Point& p)
{
    node::Widget* current_hover;
    if (m_current_mouse_hover.isAlive())
    {
        current_hover = m_current_mouse_hover.GetObjectPtr();   
    }
    else
    {
        current_hover = this->GetInteractableAt(p);
    }
    if ( m_pContextMenu!= nullptr && m_pContextMenu->GetMIHandlePtr().GetObjectPtr() != current_hover)
    {
        DestroyContextMenu();
    }
    if (current_hover)
    {
        auto result = current_hover->LMBDown(p);
        switch (result)
        {
            using enum MI::ClickEvent;
            case CAPTURE_START:
            {
                this->b_mouseCaptured = true;
                break;
            }
            case CAPTURE_END:
            {
                this->b_mouseCaptured = false;
                break;
            }
            case CLICKED:
            {
                break;
            }
            case NONE:
            {
                break;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool node::Scene::OnRMBDown(const SDL_Point& p)
{
    node::Widget* current_hover;
    if (m_current_mouse_hover.isAlive())
    {
        current_hover = m_current_mouse_hover.GetObjectPtr();   
    }
    else
    {
        current_hover = this->GetInteractableAt(p);
    }
    if ( m_pContextMenu!= nullptr && m_pContextMenu->GetMIHandlePtr().GetObjectPtr() != current_hover)
    {
        DestroyContextMenu();
    }
    if (current_hover)
    {
        auto result = current_hover->RMBDown(p);
        switch (result)
        {
            using enum MI::ClickEvent;
            case CAPTURE_START:
            {
                this->b_mouseCaptured = true;
                break;
            }
            case CAPTURE_END:
            {
                this->b_mouseCaptured = false;
                break;
            }
            case CLICKED:
            {
                break;
            }
            case NONE:
            {
                break;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool node::Scene::OnRMBUp(const SDL_Point& p)
{
    node::Widget* current_hover;
    if (m_current_mouse_hover.isAlive())
    {
        current_hover = m_current_mouse_hover.GetObjectPtr();   
    }
    else
    {
        current_hover = this->GetInteractableAt(p);
    }
    if ( m_pContextMenu!= nullptr && m_pContextMenu->GetMIHandlePtr().GetObjectPtr() != current_hover)
    {
        DestroyContextMenu();
    }
    if (current_hover)
    {
        auto result = current_hover->RMBUp(p);
        switch (result)
        {
            using enum MI::ClickEvent;
            case CAPTURE_START:
            {
                this->b_mouseCaptured = true;
                break;
            }
            case CAPTURE_END:
            {
                this->b_mouseCaptured = false;
                break;
            }
            case CLICKED:
            {
                break;
            }
            case NONE:
            {
                break;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool node::Scene::OnLMBUp(const SDL_Point& p)
{
    node::Widget* current_hover;
    if (m_current_mouse_hover.isAlive())
    {
        current_hover = m_current_mouse_hover.GetObjectPtr();
    }
    else
    {
        current_hover = this->GetInteractableAt(p);
    }
    if ( m_pContextMenu!= nullptr && m_pContextMenu->GetMIHandlePtr().GetObjectPtr() != current_hover)
    {
        DestroyContextMenu();
    }

    if (m_dragObject)
    {
        if (current_hover && current_hover->IsDropTarget())
        {
            auto&& widget = m_current_mouse_hover.GetObjectPtr();
            widget->DropObject(*m_dragObject, p);
        }
        m_dragObject = std::nullopt;
        return true;
    }

    if (current_hover)
    {
        auto result = current_hover->LMBUp(p);
        switch (result)
        {
            using enum MI::ClickEvent;
            case CAPTURE_START:
            {
                this->b_mouseCaptured = true;
                break;
            }
            case CAPTURE_END:
            {
                this->b_mouseCaptured = false;
                break;
            }
            case CLICKED:
            {
                break;
            }
            case NONE:
            {
                break;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool node::Scene::OnScroll(const double amount, SDL_Point p)
{
    if (m_current_mouse_hover.isAlive())
    {
        m_current_mouse_hover.GetObjectPtr()->Scroll(amount, p);
    }
    return false;
}

node::Scene::Scene(SDL_Rect rect, Application* parent)
:p_parent(parent), m_rect_base(rect), m_rect(rect)
{

}

node::Scene::~Scene() = default;

void node::Scene::InvalidateRect()
{
    p_parent->InvalidateRect();
}