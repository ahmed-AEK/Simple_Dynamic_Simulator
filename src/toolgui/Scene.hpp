#pragma once

#include "toolgui/MouseInteractable_interface.hpp"

#include "SDL_Framework/SDL_headers.h"
#include "toolgui/toolgui_exports.h"
#include "toolgui/Widget.hpp"
#include <vector>
#include <memory>
#include "toolgui/NodeMacros.h"
#include <optional>
#include "toolgui/DragDropObject.hpp"

namespace node
{

    class Application;
    class ContextMenu;
    class SidePanel;
    class ToolBar;
    class Dialog;
    class ToolTipWidget;

    
    class TOOLGUI_API Scene: public Widget
    {
    public:
        Scene(const WidgetSize& size, Application* parent);
        
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;

        void Draw(SDL::Renderer& renderer) override;
        virtual ~Scene();
        Application* GetApp() { return p_parent; }

        void AddNormalDialog(std::unique_ptr<node::Dialog> dialog);
        void BumpDialogToTop(const node::Dialog* dialog);
        std::unique_ptr<node::Dialog> PopDialog(const node::Dialog* dialog);
        void CloseAllDialogs();

        void SetModalDialog(std::unique_ptr<node::Dialog> dialog);
        
        virtual void OnInit() {};

        void InvalidateRect();
        void Start();

        void ShowToolTip(std::unique_ptr<ToolTipWidget> tooltip);
        void HideToolTip(Widget* widget);

        void ShowContextMenu(std::unique_ptr<node::ContextMenu> menu, const SDL_FPoint& p);
        void DestroyContextMenu();

        void SetSidePanel(std::unique_ptr<SidePanel> panel);
        void SetToolBar(std::unique_ptr<ToolBar> toolbar);
        ToolBar* GetToolBar() const { return m_toolbar.get(); }
        void SetCenterWidget(std::unique_ptr<Widget> widget);
        Widget* GetCenterWidget() { return m_centralWidget.get(); }

        void StartDragObject(DragDropObject object);
        void CancelCurrentDrag();

        void SendKeyPress(KeyboardEvent& e) { OnSendKeyPress(e); }
        void SendKeyPress(KeyboardEvent&& e) { OnSendKeyPress(e); }
        void SendCharPress(TextInputEvent& e) { OnSendChar(e); }
        void SendCharPress(TextInputEvent&& e) { OnSendChar(e); }

        Application* GetApp() const override;

        void SetFocus(Widget* widget);
        void SetFocusLater(Widget* widget);

        SDL_Color GetBGColor() const { return m_bg_color; }
        void SetBGColor(SDL_Color color) { m_bg_color = color; }
    protected:
        virtual void OnStart() {};

        virtual void OnSetSize(const WidgetSize& size);
        void OnDraw(SDL::Renderer& renderer) override;

        void OnMouseMove(MouseHoverEvent& e) override;
        MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
        MI::ClickEvent OnRMBDown(MouseButtonEvent& e) override;
        MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
        MI::ClickEvent OnRMBUp(MouseButtonEvent& e) override;
        bool OnScroll(const double amount, const SDL_FPoint& p) override;

        virtual void OnSendKeyPress(KeyboardEvent& e);
        virtual void OnSendChar(TextInputEvent& e);

        node::Widget* OnGetInteractableAtPoint(const SDL_FPoint& p) const;
        std::unique_ptr<node::ContextMenu> m_pContextMenu;
        Application* p_parent;
        SDL_FRect m_rect_base;

    private:
        std::unique_ptr<ToolTipWidget> m_tooltip;
        std::vector<std::unique_ptr<Dialog>> m_dialogs;
        std::unique_ptr<Dialog> m_modal_dialog;

        node::HandlePtr<node::Widget> m_current_mouse_hover;
        node::HandlePtr<node::Widget> m_current_keyboar_focus;

        std::unique_ptr<ToolBar> m_toolbar;
        std::unique_ptr<Widget> m_centralWidget;

        std::optional<DragDropObject> m_dragObject = std::nullopt;
        bool b_mouseCaptured = false;
        SDL_Color m_bg_color{ 255, 255, 255, 255 };
    };
}