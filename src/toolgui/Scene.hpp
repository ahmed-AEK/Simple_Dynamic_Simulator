#pragma once

#include "SDL_Framework/SDL_headers.h"
#include "toolgui/toolgui_exports.h"
#include "toolgui/Widget.hpp"
#include <vector>
#include <memory>
#include "toolgui/NodeMacros.h"
#include "toolgui/Application.hpp"
#include "toolgui/MouseInteractable.hpp"
#include "toolgui/ContextMenu.hpp"

namespace node
{
    class Application;
    
    struct SceneWidgetIterator;

    struct TOOLGUI_API WidgetSlot
    {
        std::unique_ptr<node::Widget> m_ptr;
        int z_order;
    };
    class TOOLGUI_API Scene
    {
        friend SceneWidgetIterator;
    public:
        Scene(SDL_Rect rect, Application* parent);
        void Draw(SDL_Renderer* renderer);
        void SetRect(const SDL_Rect& rect);
        virtual ~Scene() noexcept {};
        Application* GetApp() { return p_parent; }
        void AddWidget(std::unique_ptr<Widget> widget, int z_order);
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

        node::HandlePtr<node::Widget> m_current_mouse_hover;
        
        bool b_mouseCaptured = false;
    };
    struct SceneWidgetIterator
    {
        SceneWidgetIterator(Scene* scene, size_t position) : p_scene{ scene }, m_position{ position } {}
        SceneWidgetIterator operator++(int) { return SceneWidgetIterator(p_scene, m_position++); }
        SceneWidgetIterator& operator++() { m_position++; return *this; }
        node::Widget& operator*() const { return *(p_scene->m_widgets[m_position].m_ptr.get()); }
        bool operator==(SceneWidgetIterator& rhs) const { return this->m_position == rhs.m_position; }
        bool operator!=(SceneWidgetIterator& rhs) const { return this->m_position != rhs.m_position; }
        node::Widget* get() {return p_scene->m_widgets[m_position].m_ptr.get(); }
    private:
        Scene* p_scene;
        size_t m_position{};
    };
}