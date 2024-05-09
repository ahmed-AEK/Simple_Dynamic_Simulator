

#include "Scene.hpp"
#include "algorithm"
#include "SDL_Framework/SDL_headers.h"

void node::Scene::Draw(SDL_Renderer* renderer)
{
    OnDraw(renderer);
}

void node::Scene::OnDraw(SDL_Renderer* renderer)
{
    for (auto&& it = m_widgets.rbegin(); it != m_widgets.rend(); it++)
    {
        auto&& widget = *it;
        widget.m_ptr->Draw(renderer);
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
            }
            if (current_hover)
            {
                current_hover->MouseIn();
            }
            if (current_hover)
            {
                m_current_mouse_hover = current_hover->GetMIHandlePtr();
            }
            else
            {
                m_current_mouse_hover = node::HandlePtr<node::Widget>{};
            }
        }
        
    }
    if (m_current_mouse_hover.isAlive())
    {
        if (auto object = m_current_mouse_hover.GetObjectPtr())
        {
            object->MouseMove({p.x, p.y});
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
    auto iter = std::lower_bound(m_widgets.begin(), m_widgets.end(), slot, [](auto& obj1, auto& obj2) {return obj1.z_order > obj2.z_order;} );
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

void node::Scene::InvalidateRect()
{
    p_parent->InvalidateRect();
}