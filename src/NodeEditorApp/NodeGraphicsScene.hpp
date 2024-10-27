#pragma once

#include "GraphicsScene/GraphicsScene.hpp"

namespace node
{
    class NodeGraphicsScene: public node::GraphicsScene
    {
    public:
        NodeGraphicsScene(SDL_Rect rect, node::Scene* parent);
        void SetDotSpacing(int new_spacing) { m_dotspace = new_spacing; }
        int GetDotSpacing() const { return m_dotspace; }
        virtual void Draw(SDL_Renderer* renderer) override;
    protected:
        virtual MI::ClickEvent OnRMBUp(MouseButtonEvent& e) override;
        virtual void OnMouseMove(MouseHoverEvent& e) override;
        void DrawDots(SDL_Renderer* renderer) const;
        void DrawCurrentInsertMode(SDL_Renderer* renderer) const;
    private:
        void DrawCoords(SDL_Renderer* renderer) const;
        int m_dotspace = 20;
        model::Point m_current_hover_point{};
        SDL_Point m_screen_hover_point{};

    };
}