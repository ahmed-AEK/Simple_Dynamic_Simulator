#pragma once

#include "toolgui/GraphicsScene.hpp"
#include "NodeEditorApp/NodeGraphicsScene.hpp"

namespace node
{
    class MainNodeScene: public node::Scene
    {
    public:
        MainNodeScene(SDL_Rect rect, node::Application* parent);
        NodeGraphicsScene* GetNodeScene() { return m_graphicsScene; }
    protected:
        virtual bool OnRMBUp(const SDL_Point& p) override;
        NodeGraphicsScene* m_graphicsScene = nullptr;
    };

}