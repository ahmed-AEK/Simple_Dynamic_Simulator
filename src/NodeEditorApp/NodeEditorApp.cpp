#include "NodeEditorApp.hpp"
#include "NodeEditorApp/MainNodeScene.hpp"
#include "NodeEditorApp/NodeGraphicsScene.hpp"

namespace node
{
    NodeEditorApplication::NodeEditorApplication()
    : Application(800,600, "Node Editor")
    {
        std::unique_ptr<Scene> scene = std::make_unique<MainNodeScene>(getRect(), this);
        SetScene(std::move(scene));
    }

    MainNodeScene* NodeEditorApplication::GetScene() noexcept
    {
        return static_cast<MainNodeScene*>(Application::GetScene());
    }

    bool NodeEditorApplication::HandleEvent(SDL_Event& e)
    {
        switch (e.type)
        {
            case SDL_KEYDOWN:
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_RCTRL:
                case SDLK_LCTRL:
                    if (GetScene() && GetScene()->GetNodeScene())
                    {
                        GetScene()->GetNodeScene()->SetMode(GraphicsSceneMode::Insert);
                    }
                    return true;
                case SDLK_RSHIFT:
                case SDLK_LSHIFT:
                    if (GetScene() && GetScene()->GetNodeScene())
                    {
                        GetScene()->GetNodeScene()->SetMode(GraphicsSceneMode::Delete);
                    }
                    return true;
                case SDLK_ESCAPE:
                    if (GetScene())
                    {
                        GetScene()->CancelCurrentLogic();
                    }
                    if (GetScene() && GetScene()->GetNodeScene())
                    {
                        GetScene()->GetNodeScene()->CancelCurrentLogic();
                    }
                    return true;
                }
                break;
            }
            case SDL_KEYUP:
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_RCTRL:
                case SDLK_LCTRL:
                case SDLK_RSHIFT:
                case SDLK_LSHIFT:
                    if (GetScene() && GetScene()->GetNodeScene())
                    {
                        GetScene()->GetNodeScene()->SetMode(GraphicsSceneMode::Normal);
                    }
                    return true;
                }
                break;
            }
        }
        return Application::HandleEvent(e);
    }
}
