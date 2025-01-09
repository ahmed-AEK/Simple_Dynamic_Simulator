

#include "Scene.hpp"
#include "algorithm"
#include "SDL_Framework/SDL_headers.h"
#include "SDL_Framework/SDL_Math.hpp"
#include "ContextMenu.hpp"
#include "Application.hpp"
#include "Widget.hpp"
#include "SidePanel.hpp"
#include "ToolBar.hpp"
#include "Dialog.hpp"
#include "ToolTipWidget.hpp"

void node::Scene::Draw(SDL::Renderer& renderer)
{
    OnDraw(renderer);
    if (m_current_mouse_hover.isAlive() && m_dragObject)
    {
        SDL_FPoint p;
        SDL_GetMouseState(&p.x, &p.y);
        auto clip = renderer.ClipRect(WidgetRect(*m_current_mouse_hover.GetObjectPtr()));
        m_current_mouse_hover.GetObjectPtr()->DrawDropObject(renderer, *m_dragObject, p);
    }
}

void node::Scene::OnDraw(SDL::Renderer& renderer)
{
    if (m_centralWidget)
    {
        auto clip = renderer.ClipRect(WidgetRect(*m_centralWidget));
        m_centralWidget->Draw(renderer);
    }
    if (m_toolbar)
    {
        auto clip = renderer.ClipRect(WidgetRect(*m_toolbar));
        m_toolbar->Draw(renderer);
    }
    if (m_sidePanel)
    {
        auto clip = renderer.ClipRect(WidgetRect(*m_sidePanel));
        m_sidePanel->Draw(renderer);
    }
    for (auto&& it = m_dialogs.begin(); it != m_dialogs.end(); it++)
    {
        auto&& widget = *it;
        auto clip = renderer.ClipRect(WidgetRect(*widget));
        widget->Draw(renderer);
    }
    if (m_modal_dialog)
    {
        auto clip = renderer.ClipRect(WidgetRect(*m_modal_dialog));
        m_modal_dialog->Draw(renderer);
    }
    if (m_tooltip)
    {
        auto clip = renderer.ClipRect(WidgetRect(*m_tooltip));
        m_tooltip->Draw(renderer);
    }
    if (m_pContextMenu)
    {
        auto clip = renderer.ClipRect(WidgetRect(*m_pContextMenu));
        m_pContextMenu->Draw(renderer);
    }
}

void node::Scene::ShowContextMenu(std::unique_ptr<node::ContextMenu> menu, const SDL_FPoint& p)
{
    m_pContextMenu = std::move(menu);
    m_pContextMenu->SetPosition({p.x,p.y});

}
void node::Scene::DestroyContextMenu()
{
    m_pContextMenu.reset(nullptr);
}

void node::Scene::SetSidePanel(std::unique_ptr<SidePanel> panel)
{
    m_sidePanel = std::move(panel); 
}

void node::Scene::SetToolBar(std::unique_ptr<ToolBar> toolbar)
{
    m_toolbar = std::move(toolbar);
    m_toolbar->SetParent(this);
    m_toolbar->SetPosition({ 0,0 });
}

void node::Scene::SetCenterWidget(std::unique_ptr<Widget> widget)
{
    m_centralWidget = std::move(widget);
}

void node::Scene::StartDragObject(DragDropObject object)
{
    m_dragObject = std::move(object);
    if (m_current_mouse_hover.isAlive() && m_current_mouse_hover.GetObjectPtr()->IsDropTarget())
    {
        m_current_mouse_hover.GetObjectPtr()->DropEnter(*m_dragObject);
        SDL_FPoint p;
        SDL_GetMouseState(&p.x, &p.y);
        m_current_mouse_hover.GetObjectPtr()->DropHover(*m_dragObject, p);
    }
    
}

void node::Scene::CancelCurrentDrag()
{
    if (m_dragObject)
    {
        if (auto ptr = m_current_mouse_hover.GetObjectPtr())
        {
            ptr->DropExit(*m_dragObject);
        }
        m_dragObject = std::nullopt;
    }
    b_mouseCaptured = false;
    for (auto&& dialog_obj : m_dialogs)
    {
        if (dialog_obj->BeingDragged())
        {
            dialog_obj->StopDrag();
        }
    }
}

node::Application* node::Scene::GetApp() const
{
    return p_parent;
}

void node::Scene::SetFocus(Widget* widget)
{
    Widget* focusable = nullptr;
    if (widget)
    {
        focusable = widget->GetFocusable();
    }
    // change fous if needed
    Widget* current_focus = m_current_keyboar_focus.GetObjectPtr();
    if (current_focus != focusable)
    {
        if (current_focus)
        {
            m_current_keyboar_focus.GetObjectPtr()->SetFocused(false);
            m_current_keyboar_focus = nullptr;
        }
        if (focusable)
        {
            m_current_keyboar_focus = focusable->GetMIHandlePtr();
            focusable->SetFocused(true);
        }
    }
}

void node::Scene::OnMouseMove(MouseHoverEvent& e)
{
    SDL_FPoint p{ e.globalPosition() };
    if (!b_mouseCaptured)
    {
        node::Widget* current_hover = this->OnGetInteractableAtPoint(p);
        node::Widget* old_hover = nullptr;
        if (m_current_mouse_hover.isAlive())
        {
            old_hover = m_current_mouse_hover.GetObjectPtr();
        }
        if (current_hover != old_hover)
        {
            if (old_hover)
            {
                auto global_widget_pos = old_hover->GetGlobalPosition();
                e.local_position = e.globalPosition() - global_widget_pos;
                old_hover->MouseOut(e);
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
                auto global_widget_pos = current_hover->GetGlobalPosition();
                e.local_position = e.globalPosition() - global_widget_pos;
                current_hover->MouseIn(e);
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
                auto global_widget_pos = m_current_mouse_hover->GetGlobalPosition();
                e.local_position = e.globalPosition() - global_widget_pos;
                object->MouseMove(e);
            }
        }
    }
}


node::Widget* node::Scene::OnGetInteractableAtPoint(const SDL_FPoint& p) const
{
    if (m_pContextMenu)
    {
        if (auto interactable = m_pContextMenu->GetInteractableAtPoint(p - m_pContextMenu->GetPosition()))
        {
            return interactable;
        }
    }
    if (m_modal_dialog)
    {
        node::Widget* current_hover = m_modal_dialog->GetInteractableAtPoint(p - m_modal_dialog->GetPosition());
        return current_hover;
    }
    for (auto it = m_dialogs.rbegin(); it != m_dialogs.rend(); it++)
    {
        auto&& widget = *it;
        node::Widget* current_hover = widget->GetInteractableAtPoint(p - widget->GetPosition());
        if (current_hover)
        {
            return current_hover;
        }
    }
    if (m_sidePanel)
    {
        if (auto result = m_sidePanel->GetInteractableAtPoint(p - m_sidePanel->GetPosition()))
        {
            return result;
        }
    }
    if (m_toolbar)
    {
        if (auto result = m_toolbar->GetInteractableAtPoint(p - m_toolbar->GetPosition()))
        {
            return result;
        }
    }
    if (m_centralWidget)
    {
        if (auto result = m_centralWidget->GetInteractableAtPoint(p - m_centralWidget->GetPosition()))
        {
            return result;
        }
    }
    return nullptr;
}

void node::Scene::AddNormalDialog(std::unique_ptr<node::Dialog> dialog)
{
    m_dialogs.push_back(std::move(dialog));
}

void node::Scene::BumpDialogToTop(const node::Dialog* dialog)
{
    auto it = std::find_if(m_dialogs.begin(), m_dialogs.end(), [&](const auto& ptr) {return ptr.get() == dialog; });
    if (it != m_dialogs.end())
    {
        auto dialog_ptr = std::move(*it);
        m_dialogs.erase(it);
        m_dialogs.push_back(std::move(dialog_ptr));
        auto ptr = m_dialogs.back().get();
        SetFocus(ptr);
    }
}

std::unique_ptr<node::Dialog> node::Scene::PopDialog(const node::Dialog* dialog)
{
    auto it = std::find_if(m_dialogs.begin(), m_dialogs.end(), [&](const auto& ptr) { return ptr.get() == dialog; });
    assert(it != m_dialogs.end());
    if (it != m_dialogs.end())
    {
        auto out = std::move(*it);
        m_dialogs.erase(it);
        return out;
    }
    return nullptr;
}

void node::Scene::CloseAllDialogs()
{
    while (m_dialogs.size())
    {
        const auto* ptr = m_dialogs[0].get();
        PopDialog(ptr);
    }
    SetModalDialog(nullptr);
}

void node::Scene::SetModalDialog(std::unique_ptr<node::Dialog> dialog)
{
    m_modal_dialog = std::move(dialog);
    if (m_dragObject)
    {
        if (m_current_mouse_hover && m_current_mouse_hover->IsDropTarget())
        {
            m_current_mouse_hover->DropExit(*m_dragObject);
        }
        m_dragObject = std::nullopt;
    }
    if (m_current_mouse_hover)
    {
        MouseHoverEvent e{ {0,0} };
        m_current_mouse_hover->MouseOut(e);
    }
    m_current_mouse_hover = nullptr;
    m_current_keyboar_focus = nullptr;
    m_pContextMenu = nullptr;
    SetFocus(m_modal_dialog.get());
}

void node::Scene::Start()
{
    OnStart();
}

void node::Scene::ShowToolTip(std::unique_ptr<ToolTipWidget> tooltip)
{
    m_tooltip = std::move(tooltip);
    m_tooltip->SetParent(this);
}

void node::Scene::HideToolTip(Widget* widget)
{
    if (m_tooltip.get() == widget)
    {
        m_tooltip.reset();
    }
}

void node::Scene::OnSetSize(const WidgetSize& size)
{
    Widget::OnSetSize(size);
    if (m_sidePanel)
    {
        m_sidePanel->UpdateWindowSize(size);
    }
    if (m_toolbar)
    {
        m_toolbar->SetSize({ size.w, ToolBar::height });
    }

    if (m_centralWidget)
    {
        SDL_FPoint scene_pos{ 0, static_cast<float>(m_toolbar ? ToolBar::height : 0) };
        m_centralWidget->SetPosition(scene_pos);
        WidgetSize scene_size{ size.w, size.h - (m_toolbar ? ToolBar::height : 0) };
        m_centralWidget->SetSize(scene_size);
    }

    auto dialog_resizer = [&](Dialog& dialog)
        {
            if (Dialog::ScreenResizeStrategy::FixedPosition != dialog.GetResizeStrategy())
            {
                return;
            }
            SDL_Rect modified_rect = Widget::WidgetRect(dialog);
            SDL_FRect title_rect = dialog.GetTitleBarRect();
            if (modified_rect.x + modified_rect.w > size.w)
            {
                modified_rect.x = static_cast<int>(size.w - modified_rect.w);
            }
            if (modified_rect.x < 0)
            {
                modified_rect.x = 0;
            }
            if (modified_rect.y + title_rect.h > size.h)
            {
                modified_rect.y = static_cast<int>(size.h - title_rect.h);
            }
            if (modified_rect.y < 0)
            {
                modified_rect.y = 0;
            }
            dialog.SetPosition({ static_cast<float>(modified_rect.x), 
                static_cast<float>(modified_rect.y) });
        };

    for (auto& dialog: m_dialogs)
    {
        assert(dialog);
        dialog_resizer(*dialog);
    }
    if (m_modal_dialog)
    {
        dialog_resizer(*m_modal_dialog);
    }
}

MI::ClickEvent node::Scene::OnLMBDown(MouseButtonEvent& e)
{
    {
        MouseHoverEvent move_evt{ e.globalPosition() };
        OnMouseMove(move_evt);
    }
    node::Widget* current_hover = m_current_mouse_hover.GetObjectPtr();

    if ( m_pContextMenu && m_pContextMenu->GetMIHandlePtr().GetObjectPtr() != current_hover)
    {
        DestroyContextMenu();
    }

    if (current_hover)
    {
        Widget* current_widget = current_hover;
        bool processed = false;
        while (current_widget && !processed)
        {
            const auto& global_widget_pos = current_widget->GetGlobalPosition();
            e.local_position = e.globalPosition() - global_widget_pos;
            auto result = current_hover->LMBDown(e);
            switch (result)
            {
                using enum MI::ClickEvent;
            case CAPTURE_START:
            {
                this->b_mouseCaptured = true;
                processed = true;
                SetFocus(current_hover);
                break;
            }
            case CAPTURE_END:
            {
                this->b_mouseCaptured = false;
                processed = true;
                SetFocus(current_hover);
                break;
            }
            case CLICKED:
            {
                processed = true;
                SetFocus(current_hover);
                break;
            }
            case NONE:
            {
                current_widget = current_widget->GetParent();
                break;
            }
            }
        }
        if (processed)
        {
            return MI::ClickEvent::CLICKED;
        }
        else
        {
            return MI::ClickEvent::NONE;
        }
    }
    else
    {
        return MI::ClickEvent::NONE;
    }
}

MI::ClickEvent node::Scene::OnRMBDown(MouseButtonEvent& e)
{
    {
        MouseHoverEvent move_evt{ e.globalPosition() };
        OnMouseMove(move_evt);
    }
    node::Widget* current_hover = m_current_mouse_hover.GetObjectPtr();

    if ( m_pContextMenu && m_pContextMenu->GetMIHandlePtr().GetObjectPtr() != current_hover)
    {
        DestroyContextMenu();
    }
    if (current_hover)
    {
        const auto& global_widget_pos = current_hover->GetGlobalPosition();
        e.local_position = e.globalPosition() - global_widget_pos;
        auto result = current_hover->RMBDown(e);
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
        return MI::ClickEvent::CLICKED;
    }
    else
    {
        return MI::ClickEvent::NONE;
    }
}

MI::ClickEvent node::Scene::OnRMBUp(MouseButtonEvent& e)
{
    node::Widget* current_hover = m_current_mouse_hover.GetObjectPtr();

    if ( m_pContextMenu && m_pContextMenu->GetMIHandlePtr().GetObjectPtr() != current_hover)
    {
        DestroyContextMenu();
    }
    if (current_hover)
    {
        const auto& global_widget_pos = current_hover->GetGlobalPosition();
        e.local_position = e.globalPosition() - global_widget_pos;
        auto result = current_hover->RMBUp(e);
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
        return MI::ClickEvent::CLICKED;
    }
    else
    {
        return MI::ClickEvent::NONE;
    }
}

MI::ClickEvent node::Scene::OnLMBUp(MouseButtonEvent& e)
{
    SDL_FPoint p{ e.globalPosition() };
    node::Widget* current_hover = m_current_mouse_hover.GetObjectPtr();

    if ( m_pContextMenu && m_pContextMenu->GetMIHandlePtr().GetObjectPtr() != current_hover)
    {
        DestroyContextMenu();
    }

    if (m_dragObject)
    {
        if (current_hover && current_hover->IsDropTarget())
        {
            auto&& widget = m_current_mouse_hover.GetObjectPtr();
            widget->DropObject(*m_dragObject, p - widget->GetGlobalPosition());
            SetFocus(widget);
        }
        m_dragObject = std::nullopt;
        return MI::ClickEvent::CLICKED;
    }

    if (current_hover)
    {
        const auto& global_widget_pos = current_hover->GetGlobalPosition();
        e.local_position = e.globalPosition() - global_widget_pos;
        auto result = current_hover->LMBUp(e);
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
        return MI::ClickEvent::CLICKED;
    }
    else
    {
        return MI::ClickEvent::NONE;
    }
}

bool node::Scene::OnScroll(const double amount, const SDL_FPoint& p)
{
    if (m_current_mouse_hover.isAlive())
    {
        m_current_mouse_hover.GetObjectPtr()->Scroll(amount, p - m_current_mouse_hover->GetGlobalPosition());
    }
    return false;
}

void node::Scene::OnSendKeyPress(KeyboardEvent& e)
{
    if (m_current_keyboar_focus)
    {
        Widget* current_widget = m_current_keyboar_focus.GetObjectPtr();
        bool processed = false;
        while (current_widget && !processed)
        {
            processed = current_widget->KeyPress(e);
            current_widget = current_widget->GetParent();
        }
    }
}

void node::Scene::OnSendChar(TextInputEvent& e)
{
    if (m_current_keyboar_focus)
    {
        Widget* current_widget = m_current_keyboar_focus.GetObjectPtr();
        bool processed = false;
        while (current_widget && !processed)
        {
            processed = current_widget->CharPress(e);
            current_widget = current_widget->GetParent();
        }
    }
}

node::Scene::Scene(const WidgetSize& size, Application* parent)
    :Widget{size, nullptr}, p_parent(parent), m_rect_base(size.ToRect())
{

}

node::Scene::~Scene() = default;

void node::Scene::InvalidateRect()
{
    p_parent->InvalidateRect();
}
