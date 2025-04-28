#pragma once

#include "GraphicsScene/GraphicsScene.hpp"

namespace node
{
    class ToolsManager;
    class NodeGraphicsScene: public node::GraphicsScene
    {
    public:
        NodeGraphicsScene(const WidgetSize& size, node::Widget* parent);
        void SetDotSpacing(int new_spacing) { m_dotspace = new_spacing; }
        int GetDotSpacing() const { return m_dotspace; }
        void SetToolsManager(std::weak_ptr<ToolsManager> manager);
        void SetObjectsManager(std::weak_ptr<GraphicsObjectsManager> manager);
    protected:
        virtual MI::ClickEvent OnRMBUp(MouseButtonEvent& e) override;
        virtual void OnMouseMove(MouseHoverEvent& e) override;
        void DrawDots(SDL_Renderer* renderer) const;
        void DrawCurrentInsertMode(SDL_Renderer* renderer) const;
        bool OnKeyPress(KeyboardEvent& e) override;
        void OnKeyboardFocusIn() override;
        void OnDraw(SDL::Renderer& renderer) override;
    private:
        void DrawCoords(SDL_Renderer* renderer) const;
        bool DeleteSelection();
        int m_dotspace = 20;
        model::Point m_current_hover_point{};
        SDL_FPoint m_screen_hover_point{};
        std::weak_ptr<ToolsManager> m_tools_manager;
        std::weak_ptr<GraphicsObjectsManager> m_objects_manager;
    };
}