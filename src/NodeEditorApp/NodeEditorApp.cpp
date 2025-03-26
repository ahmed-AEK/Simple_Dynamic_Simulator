#include "NodeEditorApp.hpp"
#include "NodeEditorApp/MainNodeScene.hpp"
#include "NodeEditorApp/NodeGraphicsScene.hpp"
#include "GraphicsScene/ToolsManager.hpp"
#include "SDL_Framework/Utility.hpp"

namespace node
{
    NodeEditorApplication::NodeEditorApplication()
    : Application(800,600, "Dynamic Simulator")
    {
        std::unique_ptr<MainNodeScene> scene = std::make_unique<MainNodeScene>(
            WidgetSize{ static_cast<float>(getRect().w), static_cast<float>(getRect().h) }, this);
        m_scene = scene.get();
        SetScene(std::move(scene));
    }
}
