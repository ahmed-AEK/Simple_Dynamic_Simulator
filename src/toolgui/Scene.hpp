#pragma once

#include "toolgui/MouseInteractable_interface.hpp"

#include "SDL_Framework/SDL_headers.h"
#include "toolgui/toolgui_exports.h"
#include <vector>
#include <memory>
#include "toolgui/NodeMacros.h"
#include <optional>
#include "toolgui/DragDropObject.hpp"

namespace node
{

    class Application;
    class Widget;
    class ContextMenu;
    class SidePanel;
    class ToolBar;
    class Dialog;

    class TOOLGUI_API Scene
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

        void SetModalDialog(std::unique_ptr<node::Dialog> dialog);
        
        virtual void OnInit() {};
        virtual void MouseMove(const SDL_Point& p) {OnMouseMove(p);}
        virtual void LMBDown(const SDL_Point& p) {OnLMBDown(p);}
        virtual void RMBDown(const SDL_Point& p) {OnRMBDown(p);}
        virtual void LMBUp(const SDL_Point& p) {OnLMBUp(p);}
        virtual void RMBUp(const SDL_Point& p) {OnRMBUp(p);}
        virtual void Scroll(const double amount, SDL_Point p) {OnScroll(amount, p);}

        void InvalidateRect();
        void Start();

        void ShowContextMenu(std::unique_ptr<node::ContextMenu> menu, const SDL_Point& p);
        void DestroyContextMenu();

        void SetSidePanel(std::unique_ptr<SidePanel> panel);
        void SetToolBar(std::unique_ptr<ToolBar> toolbar);
        void SetgScene(std::unique_ptr<Widget> scene);

        void StartDragObject(DragDropObject object);
        void CancelCurrentLogic();

        void KeyPress(int32_t key) { OnKeyPress(key); }
        void CharPress(int32_t key) { OnChar(key); }
    protected:
        virtual void OnStart() {};

        virtual void OnSetRect(const SDL_Rect& rect);
        virtual void OnDraw(SDL_Renderer* renderer);

        virtual void OnMouseMove(const SDL_Point& p);
        virtual bool OnLMBDown(const SDL_Point& p);
        virtual bool OnRMBDown(const SDL_Point& p);
        virtual bool OnLMBUp(const SDL_Point& p);
        virtual bool OnRMBUp(const SDL_Point& p);
        virtual bool OnScroll(const double amount, SDL_Point p);

        virtual void OnKeyPress(int32_t key);
        virtual void OnChar(int32_t key);

        virtual node::Widget* GetInteractableAt(const SDL_Point& p) const;
        std::unique_ptr<node::ContextMenu> m_pContextMenu;
        Application* p_parent;
        SDL_Rect m_rect_base;
        SDL_Rect m_rect;

    private:
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