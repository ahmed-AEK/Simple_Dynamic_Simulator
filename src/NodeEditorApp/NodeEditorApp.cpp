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
        std::unique_ptr<MainNodeScene> scene = std::make_unique<MainNodeScene>(ToFRect(getRect()), this);
        m_scene = scene.get();
        SetScene(std::move(scene));
    }

    MainNodeScene* NodeEditorApplication::GetScene() noexcept
    {
        return m_scene;
    }
}
