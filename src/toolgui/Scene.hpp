#pragma once

#include "toolgui/MouseInteractable.hpp"

#include "SDL_Framework/SDL_headers.h"
#include "toolgui/toolgui_exports.h"
#include <vector>
#include <memory>
#include "toolgui/NodeMacros.h"
#include <functional>
#include <optional>
#include "toolgui/DragDropObject.hpp"

namespace node
{
    class Application;
    class Widget;
    struct SceneWidgetIterator;
    class ContextMenu;
    class SidePanel;
    class ToolBar;

    struct TOOLGUI_API WidgetSlot
    {
        std::unique_ptr<node::Widget> m_ptr;
        int z_order;
    };

    struct TOOLGUI_API UpdateTask
    {
        node::HandlePtr<node::Widget> widget;
        std::function<void()> task;
    };

    class TOOLGUI_API Scene
    {
        friend SceneWidgetIterator;
    public:
        Scene(SDL_Rect rect, Application* parent);
        void Draw(SDL_Renderer* renderer);
        void SetRect(const SDL_Rect& rect);
        virtual ~Scene();
        Application* GetApp() { return p_parent; }
        void AddWidget(std::unique_ptr<Widget> widget, int z_order);

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

        SceneWidgetIterator begin();
        SceneWidgetIterator end();

        void SetSidePanel(std::unique_ptr<SidePanel> panel);
        void SetToolBar(std::unique_ptr<ToolBar> toolbar);
        void SetgScene(std::unique_ptr<Widget> scene);
        
        bool UpdateTasksEmpty() { return m_updateTasks.empty() && m_new_updateTasks.empty(); }
        void DoUpdateTasks();
        int64_t AddUpdateTask(UpdateTask task);
        void RemoveUpdateTask(int64_t task_id);

        void StartDragObject(DragDropObject object);
        void CancelCurrentLogic();
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

        virtual node::Widget* GetInteractableAt(const SDL_Point& p) const;
        std::unique_ptr<node::ContextMenu> m_pContextMenu;
        Application* p_parent;
        SDL_Rect m_rect_base;
        SDL_Rect m_rect;
        std::vector<WidgetSlot> m_widgets;

    private:

        node::HandlePtr<node::Widget> m_current_mouse_hover;
                
        std::unique_ptr<SidePanel> m_sidePanel;
        std::unique_ptr<ToolBar> m_toolbar;
        std::unique_ptr<Widget> m_gScene;

        std::unordered_map<int64_t, UpdateTask> m_updateTasks;
        std::unordered_map<int64_t, UpdateTask> m_new_updateTasks;
        std::vector<int64_t> m_deleted_updateTasks;
        int64_t m_current_task_id = 0;

        std::optional<DragDropObject> m_dragObject = std::nullopt;
        bool b_mouseCaptured = false;
    };

    struct SceneWidgetIterator
    {
        SceneWidgetIterator(Scene* scene, size_t position) : p_scene{ scene }, m_position{ position } {}
        SceneWidgetIterator operator++(int) { return SceneWidgetIterator(p_scene, m_position++); }
        SceneWidgetIterator& operator++() { m_position++; return *this; }
        node::Widget& operator*() const { return *(p_scene->m_widgets[m_position].m_ptr.get()); }
        bool operator==(const SceneWidgetIterator& rhs) const { return this->m_position == rhs.m_position; }
        bool operator!=(const SceneWidgetIterator& rhs) const { return this->m_position != rhs.m_position; }
        node::Widget* get() {return p_scene->m_widgets[m_position].m_ptr.get(); }
    private:
        Scene* p_scene;
        size_t m_position{};
    };
}