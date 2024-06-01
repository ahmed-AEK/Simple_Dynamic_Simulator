#pragma once

#include "toolgui/Scene.hpp"

namespace node
{

class Application;
class NodeGraphicsScene;

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