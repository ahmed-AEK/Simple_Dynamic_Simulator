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
        Scene(SDL_Rect rect, Application* parent);
        
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;

        void Draw(SDL_Renderer* renderer);
        void SetRect(const SDL_Rect& rect);
        SDL_Rect GetRect() const { return m_rect; }
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

        void ShowContextMenu(std::unique_ptr<node::ContextMenu> menu, const SDL_Point& p);
        void DestroyContextMenu();

        void SetSidePanel(std::unique_ptr<SidePanel> panel);
        void SetToolBar(std::unique_ptr<ToolBar> toolbar);
        void SetCenterWidget(std::unique_ptr<Widget> scene);

        void StartDragObject(DragDropObject object);
        void CancelCurrentLogic();

        void SendKeyPress(KeyboardEvent& e) { OnSendKeyPress(e); }
        void SendKeyPress(KeyboardEvent&& e) { OnSendKeyPress(e); }
        void SendCharPress(TextInputEvent& e) { OnSendChar(e); }
        void SendCharPress(TextInputEvent&& e) { OnSendChar(e); }

        Application* GetApp() const override;

        void SetFocus(Widget* widget);
    protected:
        virtual void OnStart() {};

        virtual void OnSetRect(const SDL_Rect& rect);
        virtual void OnDraw(SDL_Renderer* renderer);

        void OnMouseMove(MouseHoverEvent& e) override;
        MI::ClickEvent OnLMBDown(MouseButtonEvent& e) override;
        MI::ClickEvent OnRMBDown(MouseButtonEvent& e) override;
        MI::ClickEvent OnLMBUp(MouseButtonEvent& e) override;
        MI::ClickEvent OnRMBUp(MouseButtonEvent& e) override;
        bool OnScroll(const double amount, const SDL_Point& p) override;

        virtual void OnSendKeyPress(KeyboardEvent& e);
        virtual void OnSendChar(TextInputEvent& e);

        node::Widget* OnGetInteractableAtPoint(const SDL_Point& p) const;
        std::unique_ptr<node::ContextMenu> m_pContextMenu;
        Application* p_parent;
        SDL_Rect m_rect_base;
        SDL_Rect m_rect;

    private:
        std::unique_ptr<ToolTipWidget> m_tooltip;
        std::vector<std::unique_ptr<Dialog>> m_dialogs;
        std::unique_ptr<Dialog> m_modal_dialog;

        node::HandlePtr<node::Widget> m_current_mouse_hover;
        node::HandlePtr<node::Widget> m_current_keyboar_focus;

        std::unique_ptr<SidePanel> m_sidePanel;
        std::unique_ptr<ToolBar> m_toolbar;
        std::unique_ptr<Widget> m_gScene;

        std::optional<DragDropObject> m_dragObject = std::nullopt;
        bool b_mouseCaptured = false;
    };
}