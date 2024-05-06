#include "toolgui/Application.hpp"
#include "NodeEditorApp/MainNodeScene.hpp"

namespace node
{
    class NodeEditorApplication : public node::Application
    {
    public:
        NodeEditorApplication();
        MainNodeScene* GetScene() { return static_cast<MainNodeScene*>(Application::GetScene()); }
    protected:
        bool HandleEvent(SDL_Event& e) override;
    };
}